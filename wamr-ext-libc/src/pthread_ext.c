#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "../internal/wamr_ext_syscall.h"

struct tls_data {
    char thread_name[32];
};

_Thread_local struct tls_data __pthread_ext_tls_data;

int pthread_setname_np(const char *name) {
    if (!name)
        return EINVAL;
    snprintf(__pthread_ext_tls_data.thread_name, sizeof(__pthread_ext_tls_data.thread_name), "%s", name);
    // Set host thread name for debugging
    wamr_ext_syscall_arg argv[] = {
        {.p = (void*)name},
    };
    __imported_wamr_ext_syscall(__EXT_SYSCALL_PTHREAD_HOST_SETNAME, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
    return 0;
}

int pthread_getname_np(char *name, size_t len) {
    snprintf(name, len, "%s", __pthread_ext_tls_data.thread_name);
    return 0;
}
