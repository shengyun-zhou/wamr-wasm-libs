#include <wamr_ext.h>
#include <stdint.h>
#include <errno.h>
#include "../internal/wamr_ext_syscall.h"

int wamr_ext_sysctl(const char* name, void* buf, unsigned int* buflen) {
    if (!name || !buf || !buflen) {
        errno = EINVAL;
        return -1;
    }
    wamr_ext_syscall_arg argv[] = {
        {.p = (char*)name},
        {.p = buf},
        {.p = buflen}
    };
    int err = __imported_wamr_ext_syscall(__EXT_SYSCALL_WAMR_EXT_SYSCTL, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);

    if (err != 0) {
        errno = err;
        return -1;
    }
    return 0;
}
