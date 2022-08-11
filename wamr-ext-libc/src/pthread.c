#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/time.h>
#include "../internal/tls_data.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-attributes"
#define PTHREAD_EXT_MODULE "pthread_ext"

_Thread_local struct tls_data __g_tls_data;

int32_t __imported_pthread_create(uint32_t*, void*, void*, void*) __attribute__((
    __import_name__("pthread_create")
));

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void*(*func)(void*), void* arg) {
    return __imported_pthread_create(thread, (void*)attr, func, arg);
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

uint64_t __pthread_get_timeout_us(const struct timespec* abstime) {
    uint64_t abs_ts = (uint64_t)abstime->tv_sec * 1000000 + abstime->tv_nsec / 1000;
    struct timespec now = {0};
    clock_gettime(CLOCK_REALTIME, &now);
    uint64_t now_ts = (uint64_t)now.tv_sec * 1000000 + now.tv_nsec / 1000;
    if (abs_ts >= now_ts)
        return abs_ts - now_ts;
    return 0;
}

int32_t __imported_pthread_mutex_init(uint32_t*, void*) __attribute__((
    __import_module__(PTHREAD_EXT_MODULE),
    __import_name__("pthread_mutex_init")
));

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr) {
    return __imported_pthread_mutex_init(mutex, (void*)attr);
}

int32_t __imported_pthread_mutex_lock(uint32_t*) __attribute__((
    __import_module__(PTHREAD_EXT_MODULE),
    __import_name__("pthread_mutex_lock")
));

int pthread_mutex_lock(pthread_mutex_t *mutex) {
    return __imported_pthread_mutex_lock(mutex);
}

int32_t __imported_pthread_mutex_unlock(uint32_t*) __attribute__((
    __import_module__(PTHREAD_EXT_MODULE),
    __import_name__("pthread_mutex_unlock")
));

int pthread_mutex_unlock(pthread_mutex_t* mutex) {
    return __imported_pthread_mutex_unlock(mutex);
}

int32_t __imported_pthread_mutex_trylock(uint32_t*) __attribute__((
    __import_module__(PTHREAD_EXT_MODULE),
    __import_name__("pthread_mutex_trylock")
));

int pthread_mutex_trylock(pthread_mutex_t* mutex) {
    return __imported_pthread_mutex_trylock(mutex);
}

int32_t __imported_pthread_mutex_timedlock(uint32_t*, uint64_t) __attribute__((
    __import_module__(PTHREAD_EXT_MODULE),
    __import_name__("pthread_mutex_timedlock")
));

int pthread_mutex_timedlock(pthread_mutex_t* mutex, const struct timespec* abstime) {
    if (!abstime)
        return pthread_mutex_lock(mutex);
    return __imported_pthread_mutex_timedlock(mutex, __pthread_get_timeout_us(abstime));
}

int32_t __imported_pthread_mutex_destroy(uint32_t*) __attribute__((
    __import_module__(PTHREAD_EXT_MODULE),
    __import_name__("pthread_mutex_destroy")
));

int pthread_mutex_destroy(pthread_mutex_t* mutex) {
    return __imported_pthread_mutex_destroy(mutex);
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

int32_t __imported_pthread_cond_init(uint32_t*, void*) __attribute__((
    __import_module__(PTHREAD_EXT_MODULE),
    __import_name__("pthread_cond_init")
));

int pthread_cond_init(pthread_cond_t* cond, const pthread_condattr_t* attr) {
    return __imported_pthread_cond_init(cond, (void*)attr);
}

int32_t __imported_pthread_cond_destroy(uint32_t*) __attribute__((
    __import_module__(PTHREAD_EXT_MODULE),
    __import_name__("pthread_cond_destroy")
));

int pthread_cond_destroy(pthread_cond_t* cond) {
    return __imported_pthread_cond_destroy(cond);
}

int32_t __imported_pthread_cond_wait(uint32_t*, uint32_t*) __attribute__((
    __import_module__(PTHREAD_EXT_MODULE),
    __import_name__("pthread_cond_wait")
));

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
    return __imported_pthread_cond_wait(cond, mutex);
}

int32_t __imported_pthread_cond_timedwait(uint32_t*, uint32_t*, uint64_t) __attribute__((
    __import_module__(PTHREAD_EXT_MODULE),
    __import_name__("pthread_cond_timedwait")
));

int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime) {
    if (!abstime)
        return pthread_cond_wait(cond, mutex);
    return __imported_pthread_cond_timedwait(cond, mutex, __pthread_get_timeout_us(abstime));
}

int32_t __imported_pthread_cond_broadcast(uint32_t*) __attribute__((
    __import_module__(PTHREAD_EXT_MODULE),
    __import_name__("pthread_cond_broadcast")
));

int pthread_cond_broadcast(pthread_cond_t *cond) {
    return __imported_pthread_cond_broadcast(cond);
}

int32_t __imported_pthread_cond_signal(uint32_t*) __attribute__((
    __import_module__(PTHREAD_EXT_MODULE),
    __import_name__("pthread_cond_signal")
));

int pthread_cond_signal(pthread_cond_t *cond) {
    return __imported_pthread_cond_signal(cond);
}

int32_t __imported_pthread_rwlock_init(uint32_t*, void*) __attribute__((
    __import_module__(PTHREAD_EXT_MODULE),
    __import_name__("pthread_rwlock_init")
));

int pthread_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr) {
    return __imported_pthread_rwlock_init(rwlock, (void*)attr);
}

int32_t __imported_pthread_rwlock_destroy(uint32_t*) __attribute__((
    __import_module__(PTHREAD_EXT_MODULE),
    __import_name__("pthread_rwlock_destroy")
));

int pthread_rwlock_destroy(pthread_rwlock_t *rwlock) {
    return __imported_pthread_rwlock_destroy(rwlock);
}

int32_t __imported_pthread_rwlock_rdlock(uint32_t*) __attribute__((
    __import_module__(PTHREAD_EXT_MODULE),
    __import_name__("pthread_rwlock_rdlock")
));

int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock) {
    return __imported_pthread_rwlock_rdlock(rwlock);
}

int32_t __imported_pthread_rwlock_tryrdlock(uint32_t*) __attribute__((
    __import_module__(PTHREAD_EXT_MODULE),
    __import_name__("pthread_rwlock_tryrdlock")
));

int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock) {
    return __imported_pthread_rwlock_tryrdlock(rwlock);
}

int32_t __imported_pthread_rwlock_timedrdlock(uint32_t*, uint64_t) __attribute__((
    __import_module__(PTHREAD_EXT_MODULE),
    __import_name__("pthread_rwlock_timedrdlock")
));

int pthread_rwlock_timedrdlock(pthread_rwlock_t *rwlock, const struct timespec *abstime) {
    if (!abstime)
        return pthread_rwlock_rdlock(rwlock);
    return __imported_pthread_rwlock_timedrdlock(rwlock, __pthread_get_timeout_us(abstime));
}

int32_t __imported_pthread_rwlock_wrlock(uint32_t*) __attribute__((
    __import_module__(PTHREAD_EXT_MODULE),
    __import_name__("pthread_rwlock_wrlock")
));

int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock) {
    return __imported_pthread_rwlock_wrlock(rwlock);
}

int32_t __imported_pthread_rwlock_trywrlock(uint32_t*) __attribute__((
    __import_module__(PTHREAD_EXT_MODULE),
    __import_name__("pthread_rwlock_trywrlock")
));

int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock) {
    return __imported_pthread_rwlock_trywrlock(rwlock);
}

int32_t __imported_pthread_rwlock_timedwrlock(uint32_t*, uint64_t) __attribute__((
    __import_module__(PTHREAD_EXT_MODULE),
    __import_name__("pthread_rwlock_timedwrlock")
));

int pthread_rwlock_timedwrlock(pthread_rwlock_t *rwlock, const struct timespec *abstime) {
    if (!abstime)
        return pthread_rwlock_wrlock(rwlock);
    return __imported_pthread_rwlock_timedwrlock(rwlock, __pthread_get_timeout_us(abstime));
}

int32_t __imported_pthread_rwlock_unlock(uint32_t*) __attribute__((
    __import_module__(PTHREAD_EXT_MODULE),
    __import_name__("pthread_rwlock_unlock")
));

int pthread_rwlock_unlock(pthread_rwlock_t *rwlock) {
    return __imported_pthread_rwlock_unlock(rwlock);
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
    struct pthread_cleanup_handler_stacknode* new_node = (struct pthread_cleanup_handler_stacknode*)malloc(sizeof(struct pthread_cleanup_handler_stacknode));
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
    while (__g_tls_data.cleanup_handler_stacktop)
        pthread_cleanup_pop(1);
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

int32_t __imported_pthread_setname_np(const char*) __attribute__((
    __import_module__(PTHREAD_EXT_MODULE),
    __import_name__("pthread_setname_np")
));

int pthread_setname_np(const char *name) {
    return __imported_pthread_setname_np(name);
}

int32_t __imported_pthread_getname_np(char*, uint32_t) __attribute__((
    __import_module__(PTHREAD_EXT_MODULE),
    __import_name__("pthread_getname_np")
));

int pthread_getname_np(char *name, size_t len) {
    return __imported_pthread_getname_np(name, len);
}

#pragma clang diagnostic pop
