#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/time.h>
#include <semaphore.h>
#include "../internal/wamr_ext_syscall.h"
#include "../internal/tls_data.h"
#define hidden
#include "../../wasi-libc/libc-top-half/musl/src/internal/pthread_impl.h"

_Thread_local struct tls_data __g_tls_data;

struct pthread_cleanup_handler_stacknode {
    struct pthread_cleanup_handler_stacknode* next_node;
    void (*routine)(void*);
    void *arg;
};

void __pthread_exit_cleanup(int from_func_pthread_exit) {
    if (from_func_pthread_exit) {
        while (__g_tls_data.cleanup_handler_stacktop) {
            void (*f)(void *) = __g_tls_data.cleanup_handler_stacktop->__f;
            void *x = __g_tls_data.cleanup_handler_stacktop->__x;
            __g_tls_data.cleanup_handler_stacktop = __g_tls_data.cleanup_handler_stacktop->__next;
            f(x);
        }
    } else {
        __g_tls_data.cleanup_handler_stacktop = NULL;
    }
    while (__g_tls_data.cxx_dtor_stacktop) {
        struct cxx_dtor_stacknode* p_node = __g_tls_data.cxx_dtor_stacktop;
        __g_tls_data.cxx_dtor_stacktop = __g_tls_data.cxx_dtor_stacktop->next_node;
        p_node->dtor_func(p_node->arg);
        free(p_node);
    }
    if (__g_tls_data.thread_routine_bundle) {
        free(__g_tls_data.thread_routine_bundle);
        __g_tls_data.thread_routine_bundle = NULL;
    }
}

struct pthread_routine_bundle {
    void*(*user_func)(void*);
    void* user_arg;
};

void* __pthread_routine(void* arg) {
    struct pthread_routine_bundle* p_bundle = arg;
    __g_tls_data.thread_routine_bundle = p_bundle;
    void* ret = p_bundle->user_func(p_bundle->user_arg);
    __pthread_exit_cleanup(0);
    return ret;
}

void __attribute__((destructor)) __main_atexit() {
    __pthread_exit_cleanup(0);
}

struct wamr_create_thread_req {
    uint32_t ret_handle_id;
    void*(*start_func)(void*);
    void* start_arg;
    void* stack_addr;
    uint32_t stack_size;
    uint8_t thread_detached;
};

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void*(*func)(void*), void* arg) {
    if (!func || !thread)
        return EINVAL;
    struct pthread_routine_bundle* p_bundle = malloc(sizeof(struct pthread_routine_bundle));
    p_bundle->user_func = func;
    p_bundle->user_arg = arg;

    struct wamr_create_thread_req req = {0};
    req.start_func = __pthread_routine;
    req.start_arg = p_bundle;
    if (attr) {
        size_t stack_size = 0;
        if (pthread_attr_getstack(attr, &req.stack_addr, &stack_size) != 0)
            pthread_attr_getstacksize(attr, &stack_size);
        req.stack_size = stack_size;
        int detach_state;
        if (pthread_attr_getdetachstate(attr, &detach_state) == 0 && detach_state == PTHREAD_CREATE_DETACHED)
            req.thread_detached = 1;
    }
    if (req.stack_size == 0) {
        __acquire_ptc();
        req.stack_size = __default_stacksize;
        __release_ptc();
    }
    wamr_ext_syscall_arg argv[] = {
        {.p = &req},
    };
    int ret = __imported_wamr_ext_syscall(__EXT_SYSCALL_PTHREAD_CREATE, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
    if (ret != 0) {
        free(p_bundle);
    } else {
        *thread = req.ret_handle_id;
    }
    return ret;
}

pthread_t pthread_self() {
    uint32_t handle_id = 0;
    wamr_ext_syscall_arg argv[] = {
        {.p = &handle_id},
    };
    __imported_wamr_ext_syscall(__EXT_SYSCALL_PTHREAD_SELF, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
    return handle_id;
}

int pthread_join(pthread_t thread, void **retval) {
    uint32_t syscall_join_retval = 0;
    wamr_ext_syscall_arg argv[] = {
        {.u32 = thread},
        {.p = &syscall_join_retval},
    };
    int ret = __imported_wamr_ext_syscall(__EXT_SYSCALL_PTHREAD_JOIN, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
    if (ret == 0 && retval)
        *retval = (void*)syscall_join_retval;
    return ret;
}

int pthread_detach(pthread_t thread) {
    wamr_ext_syscall_arg argv[] = {
        {.u32 = thread},
    };
    return __imported_wamr_ext_syscall(__EXT_SYSCALL_PTHREAD_DETACH, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
}

int pthread_equal(pthread_t t1, pthread_t t2) {
    return t1 == t2;
}

uint64_t __pthread_get_timeout_us(const struct timespec* abstime) {
    uint64_t abs_ts = (uint64_t)abstime->tv_sec * 1000000 + abstime->tv_nsec / 1000;
    struct timespec now = {0};
    clock_gettime(CLOCK_REALTIME, &now);
    uint64_t now_ts = (uint64_t)now.tv_sec * 1000000 + now.tv_nsec / 1000;
    if (abs_ts >= now_ts)
        return abs_ts - now_ts;
    return 0;
}

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr) {
    int mutex_type = PTHREAD_MUTEX_DEFAULT;
    if (attr)
        pthread_mutexattr_gettype(attr, &mutex_type);
    wamr_ext_syscall_arg argv[] = {
        {.p = mutex},
        {.u32 = mutex_type},
    };
    return __imported_wamr_ext_syscall(__EXT_SYSCALL_PTHREAD_MUTEX_INIT, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
}

int __pthread_mutex_timedlock_with_timeout(pthread_mutex_t *mutex, uint64_t timeout_us) {
    wamr_ext_syscall_arg argv[] = {
        {.p = mutex},
        {.u64 = timeout_us}
    };
    return __imported_wamr_ext_syscall(__EXT_SYSCALL_PTHREAD_MUTEX_TIMEDLOCK, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
}

int pthread_mutex_lock(pthread_mutex_t *mutex) {
    return __pthread_mutex_timedlock_with_timeout(mutex, UINT64_MAX);
}

int pthread_mutex_trylock(pthread_mutex_t* mutex) {
    int err = __pthread_mutex_timedlock_with_timeout(mutex, 0);
    if (err == ETIMEDOUT)
        return EBUSY;
    return err;
}

int pthread_mutex_timedlock(pthread_mutex_t* mutex, const struct timespec* abstime) {
    if (!abstime)
        return pthread_mutex_lock(mutex);
    return __pthread_mutex_timedlock_with_timeout(mutex, __pthread_get_timeout_us(abstime));
}

int pthread_mutex_unlock(pthread_mutex_t* mutex) {
    wamr_ext_syscall_arg argv[] = {
        {.p = mutex},
    };
    return __imported_wamr_ext_syscall(__EXT_SYSCALL_PTHREAD_MUTEX_UNLOCK, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
}

int pthread_mutex_destroy(pthread_mutex_t* mutex) {
    wamr_ext_syscall_arg argv[] = {
        {.p = mutex},
    };
    return __imported_wamr_ext_syscall(__EXT_SYSCALL_PTHREAD_MUTEX_DESTROY, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
}

int pthread_cond_init(pthread_cond_t* cond, const pthread_condattr_t* attr) {
    wamr_ext_syscall_arg argv[] = {
        {.p = cond},
    };
    return __imported_wamr_ext_syscall(__EXT_SYSCALL_PTHREAD_COND_INIT, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
}

int __pthread_cond_timedwait_with_timeout(pthread_cond_t *cond, pthread_mutex_t *mutex, uint64_t timeout_us) {
    wamr_ext_syscall_arg argv[] = {
        {.p = cond},
        {.p = mutex},
        {.u64 = timeout_us}
    };
    return __imported_wamr_ext_syscall(__EXT_SYSCALL_PTHREAD_COND_TIMEDWAIT, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
    return __pthread_cond_timedwait_with_timeout(cond, mutex, UINT64_MAX);
}

int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime) {
    if (!abstime)
        return pthread_cond_wait(cond, mutex);
    return __pthread_cond_timedwait_with_timeout(cond, mutex, __pthread_get_timeout_us(abstime));
}

int pthread_cond_broadcast(pthread_cond_t *cond) {
    wamr_ext_syscall_arg argv[] = {
        {.p = cond},
    };
    return __imported_wamr_ext_syscall(__EXT_SYSCALL_PTHREAD_COND_BROADCAST, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
}

int pthread_cond_signal(pthread_cond_t *cond) {
    wamr_ext_syscall_arg argv[] = {
        {.p = cond},
    };
    return __imported_wamr_ext_syscall(__EXT_SYSCALL_PTHREAD_COND_SIGNAL, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
}

int pthread_cond_destroy(pthread_cond_t* cond) {
    wamr_ext_syscall_arg argv[] = {
        {.p = cond},
    };
    return __imported_wamr_ext_syscall(__EXT_SYSCALL_PTHREAD_COND_DESTROY, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
}

int pthread_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr) {
    wamr_ext_syscall_arg argv[] = {
        {.p = rwlock},
    };
    return __imported_wamr_ext_syscall(__EXT_SYSCALL_PTHREAD_RWLOCK_INIT, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
}

int __pthread_rwlock_timedrdlock_with_timeout(pthread_rwlock_t *rwlock, uint64_t timeout_us) {
    wamr_ext_syscall_arg argv[] = {
        {.p = rwlock},
        {.u64 = timeout_us},
    };
    return __imported_wamr_ext_syscall(__EXT_SYSCALL_PTHREAD_RWLOCK_TIMEDRDLOCK, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
}

int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock) {
    return __pthread_rwlock_timedrdlock_with_timeout(rwlock, UINT64_MAX);
}

int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock) {
    int err = __pthread_rwlock_timedrdlock_with_timeout(rwlock, 0);
    if (err == ETIMEDOUT)
        return EBUSY;
    return err;
}

int pthread_rwlock_timedrdlock(pthread_rwlock_t *rwlock, const struct timespec *abstime) {
    if (!abstime)
        return pthread_rwlock_rdlock(rwlock);
    return __pthread_rwlock_timedrdlock_with_timeout(rwlock, __pthread_get_timeout_us(abstime));
}

int __pthread_rwlock_timedwrlock_with_timeout(pthread_rwlock_t* rwlock, uint64_t timeout_us) {
    wamr_ext_syscall_arg argv[] = {
        {.p = rwlock},
        {.u64 = timeout_us},
    };
    return __imported_wamr_ext_syscall(__EXT_SYSCALL_PTHREAD_RWLOCK_TIMEDWRLOCK, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
}

int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock) {
    return __pthread_rwlock_timedwrlock_with_timeout(rwlock, UINT64_MAX);
}

int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock) {
    int err = __pthread_rwlock_timedwrlock_with_timeout(rwlock, 0);
    if (err == ETIMEDOUT)
        return EBUSY;
    return err;
}

int pthread_rwlock_timedwrlock(pthread_rwlock_t *rwlock, const struct timespec *abstime) {
    if (!abstime)
        return pthread_rwlock_wrlock(rwlock);
    return __pthread_rwlock_timedwrlock_with_timeout(rwlock, __pthread_get_timeout_us(abstime));
}

int pthread_rwlock_unlock(pthread_rwlock_t *rwlock) {
    wamr_ext_syscall_arg argv[] = {
        {.p = rwlock},
    };
    return __imported_wamr_ext_syscall(__EXT_SYSCALL_PTHREAD_RWLOCK_UNLOCK, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
}

int pthread_rwlock_destroy(pthread_rwlock_t *rwlock) {
    wamr_ext_syscall_arg argv[] = {
        {.p = rwlock},
    };
    return __imported_wamr_ext_syscall(__EXT_SYSCALL_PTHREAD_RWLOCK_DESTROY, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
}

int pthread_key_create(pthread_key_t *k, void(*deconstructor)(void*)) {
    if (!k)
        return EINVAL;
    wamr_ext_syscall_arg argv[] = {
        {.p = k},
        {.p = deconstructor}
    };
    return __imported_wamr_ext_syscall(__EXT_SYSCALL_PTHREAD_KEY_CREATE, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
}

int pthread_key_delete(pthread_key_t k) {
    wamr_ext_syscall_arg argv[] = {
        {.u32 = k},
    };
    return __imported_wamr_ext_syscall(__EXT_SYSCALL_PTHREAD_KEY_DELETE, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
}

int pthread_setspecific(pthread_key_t k, const void* obj) {
    wamr_ext_syscall_arg argv[] = {
        {.u32 = k},
        {.u32 = (uintptr_t)obj}
    };
    return __imported_wamr_ext_syscall(__EXT_SYSCALL_PTHREAD_SETSPECIFIC, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
}

void* pthread_getspecific(pthread_key_t k) {
    uint32_t val = 0;
    wamr_ext_syscall_arg argv[] = {
        {.u32 = k},
        {.p = &val}
    };
    int ret = __imported_wamr_ext_syscall(__EXT_SYSCALL_PTHREAD_GETSPECIFIC, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
    if (ret != 0)
        return NULL;
    return (void*)val;
}

int pthread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void)) {
    // fork() not supported, so do nothing
    return 0;
}

int pthread_setcancelstate(int state, int *oldstate) {
    // A thread is not cancelable
    *oldstate = PTHREAD_CANCEL_DISABLE;
    return 0;
}

int pthread_setcanceltype(int type, int *oldtype) {
    *oldtype = 0;
    return 0;
}

void _pthread_cleanup_push(struct __ptcb *cb, void (*f)(void *), void *x) {
    cb->__f = f;
    cb->__x = x;
    cb->__next = __g_tls_data.cleanup_handler_stacktop;
    __g_tls_data.cleanup_handler_stacktop = cb;
}

void _pthread_cleanup_pop(struct __ptcb *cb, int run) {
    __g_tls_data.cleanup_handler_stacktop = cb->__next;
    if (run)
        cb->__f(cb->__x);
}

void pthread_exit(void *retval) {
    __pthread_exit_cleanup(1);
    wamr_ext_syscall_arg argv[] = {
        {.u32 = (uintptr_t)retval},
    };
    __imported_wamr_ext_syscall(__EXT_SYSCALL_PTHREAD_EXIT, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
}

pthread_mutex_t __g_pthread_once_lock = PTHREAD_MUTEX_INITIALIZER;

void __attribute__((constructor)) __pthread_lib_init() {
    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&__g_pthread_once_lock, &mutex_attr);
}

int pthread_once(pthread_once_t *once_control, void(*init_routine)()) {
    if (!once_control || !init_routine)
        return EINVAL;
    if (*once_control == 0) {
        pthread_mutex_lock(&__g_pthread_once_lock);
        if (*once_control == 0) {
            init_routine();
            *once_control = 1;
        }
        pthread_mutex_unlock(&__g_pthread_once_lock);
    }
    return 0;
}

int pthread_setname_np(const char *name) {
    if (!name)
        return EINVAL;
    snprintf(__g_tls_data.thread_name, sizeof(__g_tls_data.thread_name), "%s", name);
    // Set host thread name for debugging
    wamr_ext_syscall_arg argv[] = {
        {.p = (void*)name},
    };
    __imported_wamr_ext_syscall(__EXT_SYSCALL_PTHREAD_HOST_SETNAME, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
    return 0;
}

int pthread_getname_np(char *name, size_t len) {
    snprintf(name, len, "%s", __g_tls_data.thread_name);
    return 0;
}

struct _sem_s {
    uint32_t val;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

int sem_init(sem_t *sem, int pshared, unsigned int value) {
    int err = 0;
    do {
        if (!sem) {
            err = EINVAL;
            break;
        }
        if (pshared != 0) {
            err = ENOSYS;
            break;
        }
        if (!(*sem = malloc(sizeof(struct _sem_s)))) {
            err = ENOMEM;
            break;
        }
        (*sem)->val = value;
        pthread_mutex_init(&(*sem)->mutex, NULL);
        pthread_cond_init(&(*sem)->cond, NULL);
    } while (0);
    if (err != 0) {
        errno = err;
        if (sem && *sem)
            free(*sem);
        return -1;
    }
    return 0;
}

int sem_wait(sem_t* sem) {
    if (!sem || !(*sem)) {
        errno = EINVAL;
        return -1;
    }
    struct _sem_s* p_sem = *sem;
    pthread_mutex_lock(&p_sem->mutex);
    while (1) {
        if (p_sem->val > 0) {
            p_sem->val--;
            break;
        }
        pthread_cond_wait(&p_sem->cond, &p_sem->mutex);
    }
    pthread_mutex_unlock(&p_sem->mutex);
    return 0;
}

int sem_trywait(sem_t* sem) {
    if (!sem || !(*sem)) {
        errno = EINVAL;
        return -1;
    }
    struct _sem_s* p_sem = *sem;
    int err = 0;
    pthread_mutex_lock(&p_sem->mutex);
    if (p_sem->val > 0)
        p_sem->val--;
    else
        err = EAGAIN;
    pthread_mutex_unlock(&p_sem->mutex);
    if (err != 0) {
        errno = err;
        return -1;
    }
    return 0;
}

int sem_timedwait(sem_t* sem, const struct timespec *abstime) {
    if (!abstime)
        return sem_wait(sem);
    if (!sem || !(*sem)) {
        errno = EINVAL;
        return -1;
    }
    struct _sem_s* p_sem = *sem;
    int err = 0;
    pthread_mutex_lock(&p_sem->mutex);
    while (1) {
        if (p_sem->val > 0) {
            p_sem->val--;
            break;
        }
        uint64_t timeout_us = __pthread_get_timeout_us(abstime);
        if (timeout_us <= 0) {
            err = ETIMEDOUT;
            break;
        }
        __pthread_cond_timedwait_with_timeout(&p_sem->cond, &p_sem->mutex, timeout_us);
    }
    pthread_mutex_unlock(&p_sem->mutex);
    if (err != 0) {
        errno = err;
        return -1;
    }
    return 0;
}

int sem_post(sem_t* sem) {
    if (!sem || !(*sem)) {
        errno = EINVAL;
        return -1;
    }
    struct _sem_s* p_sem = *sem;
    int err = 0;
    pthread_mutex_lock(&p_sem->mutex);
    if (p_sem->val == UINT32_MAX) {
        err = EOVERFLOW;
    } else {
        p_sem->val++;
        if (p_sem->val == 1)
            pthread_cond_signal(&p_sem->cond);
    }
    pthread_mutex_unlock(&p_sem->mutex);
    if (err != 0) {
        errno = err;
        return -1;
    }
    return 0;

}

int sem_getvalue(sem_t* sem, int* sval) {
    if (!sem || !(*sem) || !sval) {
        errno = EINVAL;
        return -1;
    }
    struct _sem_s* p_sem = *sem;
    pthread_mutex_lock(&p_sem->mutex);
    if (p_sem->val > INT32_MAX)
        *sval = INT32_MAX;
    else
        *sval = p_sem->val;
    pthread_mutex_unlock(&p_sem->mutex);
    return 0;
}

int sem_destroy(sem_t* sem) {
    if (!sem || !(*sem)) {
        errno = EINVAL;
        return -1;
    }
    struct _sem_s* p_sem = *sem;
    pthread_cond_destroy(&p_sem->cond);
    pthread_mutex_destroy(&p_sem->mutex);
    free(p_sem);
    *sem = NULL;
    return 0;
}
