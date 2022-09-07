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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-attributes"

_Thread_local struct tls_data __g_tls_data;

struct pthread_cleanup_handler_stacknode {
    struct pthread_cleanup_handler_stacknode* next_node;
    void (*routine)(void*);
    void *arg;
};

void __pthread_exit_cleanup(int from_func_pthread_exit) {
    while (__g_tls_data.cleanup_handler_stacktop) {
        // Don't execute cleanup functions if this thread don't exit by calling pthread_exit()
        pthread_cleanup_pop(from_func_pthread_exit);
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

int32_t __imported_pthread_create(uint32_t*, void*, void*, void*) __attribute__((
    __import_name__("pthread_create")
));

struct pthread_routine_bundle {
    void*(*user_func)(void*);
    void* user_arg;
};

void __pthread_routine(void* arg) {
    struct pthread_routine_bundle* p_bundle = arg;
    __g_tls_data.thread_routine_bundle = p_bundle;
    p_bundle->user_func(p_bundle->user_arg);
    __pthread_exit_cleanup(0);
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void*(*func)(void*), void* arg) {
    if (!func)
        return EINVAL;
    struct pthread_routine_bundle* p_bundle = malloc(sizeof(struct pthread_routine_bundle));
    p_bundle->user_func = func;
    p_bundle->user_arg = arg;
    int ret = __imported_pthread_create(thread, NULL, __pthread_routine, p_bundle);
    if (ret != 0)
        free(p_bundle);
    return ret;
}

int32_t __imported_pthread_detach(uint32_t) __attribute__((
    __import_name__("pthread_detach")
));

int pthread_detach(pthread_t thread) {
    return __imported_pthread_detach(thread);
}

int32_t __imported_pthread_join(uint32_t, void**) __attribute__((
    __import_name__("pthread_join")
));

int pthread_join(pthread_t thread, void **retval) {
    return __imported_pthread_join(thread, retval);
}

uint32_t __imported_pthread_self() __attribute__((
    __import_name__("pthread_self")
));

pthread_t pthread_self() {
    return __imported_pthread_self();
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

int pthread_mutexattr_init(pthread_mutexattr_t *attr) {
    memset(attr, 0, sizeof(pthread_mutexattr_t));
    return 0;
}

int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type) {
    attr->__attr = type;
    return 0;
}

int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int* out_type) {
    *out_type = attr->__attr;
    return 0;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t *attr) {
    return 0;
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

int32_t __imported_pthread_key_create(uint32_t*, void(*)(void*)) __attribute__((
    __import_name__("pthread_key_create")
));

int pthread_key_create(pthread_key_t *k, void(*deconstructor)(void*)) {
    return __imported_pthread_key_create(k, deconstructor);
}

int32_t __imported_pthread_key_delete(uint32_t) __attribute__((
    __import_name__("pthread_key_delete")
));

int pthread_key_delete(pthread_key_t k) {
    return __imported_pthread_key_delete(k);
}

void* __imported_pthread_getspecific(uint32_t) __attribute__((
    __import_name__("pthread_getspecific")
));

void* pthread_getspecific(pthread_key_t k) {
    return __imported_pthread_getspecific(k);
}

int32_t __imported_pthread_setspecific(uint32_t, void*) __attribute__((
    __import_name__("pthread_setspecific")
));

int pthread_setspecific(pthread_key_t k, const void* obj) {
    return __imported_pthread_setspecific(k, (void*)obj);
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

void pthread_cleanup_push(void (*routine)(void *), void *arg) {
    struct pthread_cleanup_handler_stacknode* new_node = malloc(sizeof(struct pthread_cleanup_handler_stacknode));
    new_node->next_node = __g_tls_data.cleanup_handler_stacktop;
    new_node->routine = routine;
    new_node->arg = arg;
    __g_tls_data.cleanup_handler_stacktop = new_node;
}

void pthread_cleanup_pop(int execute) {
    if (__g_tls_data.cleanup_handler_stacktop) {
        struct pthread_cleanup_handler_stacknode* p_node = __g_tls_data.cleanup_handler_stacktop;
        __g_tls_data.cleanup_handler_stacktop = __g_tls_data.cleanup_handler_stacktop->next_node;
        if (execute)
            p_node->routine(p_node->arg);
        free(p_node);
    }
}

void __imported_pthread_exit(void*) __attribute__((
    __import_name__("pthread_exit")
));

void pthread_exit(void *retval) {
    __pthread_exit_cleanup(1);
    __imported_pthread_exit(retval);
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
        init_routine();
        *once_control = 1;
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

int pthread_attr_init(pthread_attr_t *attr) {
    return 0;
}

int pthread_attr_destroy(pthread_attr_t *attr) {
    return 0;
}

int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize) {
    return ENOSYS;
}

int pthread_attr_getstacksize(const pthread_attr_t* attr, size_t* stacksize) {
    return ENOSYS;
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

#pragma clang diagnostic pop
