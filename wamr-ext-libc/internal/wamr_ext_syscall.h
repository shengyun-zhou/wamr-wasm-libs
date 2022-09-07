#pragma once
#include <stdint.h>

typedef union wamr_ext_syscall_arg {
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
    float f32;
    double f64;
    void* p;
    uint8_t __padding[16];
} wamr_ext_syscall_arg;

_Static_assert(sizeof(wamr_ext_syscall_arg) >= 16, "Require sizeof(wamr_ext_syscall_arg) >= 16");

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-attributes"

int32_t __imported_wamr_ext_syscall(uint32_t syscall_id, uint32_t argc, wamr_ext_syscall_arg *argv) __attribute__((
    __import_module__("wamr_ext"),
    __import_name__("syscall")
));

enum wamr_ext_syscall_id {
    __EXT_SYSCALL_WAMR_EXT_SYSCTL = 1,

    // Pthread ext
    __EXT_SYSCALL_PTHREAD_MUTEX_INIT = 100,
    __EXT_SYSCALL_PTHREAD_MUTEX_TIMEDLOCK = 101,
    __EXT_SYSCALL_PTHREAD_MUTEX_UNLOCK = 102,
    __EXT_SYSCALL_PTHREAD_MUTEX_DESTROY = 103,

    __EXT_SYSCALL_PTHREAD_COND_INIT = 110,
    __EXT_SYSCALL_PTHREAD_COND_TIMEDWAIT = 111,
    __EXT_SYSCALL_PTHREAD_COND_BROADCAST = 112,
    __EXT_SYSCALL_PTHREAD_COND_SIGNAL = 113,
    __EXT_SYSCALL_PTHREAD_COND_DESTROY = 114,

    __EXT_SYSCALL_PTHREAD_RWLOCK_INIT = 120,
    __EXT_SYSCALL_PTHREAD_RWLOCK_TIMEDRDLOCK = 121,
    __EXT_SYSCALL_PTHREAD_RWLOCK_TIMEDWRLOCK = 122,
    __EXT_SYSCALL_PTHREAD_RWLOCK_UNLOCK = 123,
    __EXT_SYSCALL_PTHREAD_RWLOCK_DESTROY = 124,

    __EXT_SYSCALL_PTHREAD_HOST_SETNAME = 130,

    // Filesystem ext
    __EXT_SYSCALL_FD_STATVFS = 200,
    __EXT_SYSCALL_FD_EXT_FCNTL = 201,

    // Socket ext
    __EXT_SYSCALL_SOCK_OPEN = 300,
    __EXT_SYSCALL_SOCK_BIND = 301,
    __EXT_SYSCALL_SOCK_CONNECT = 302,
    __EXT_SYSCALL_SOCK_LISTEN = 303,
    __EXT_SYSCALL_SOCK_ACCEPT = 304,
    __EXT_SYSCALL_SOCK_GETSOCKNAME = 305,
    __EXT_SYSCALL_SOCK_GETPEERNAME = 306,
    __EXT_SYSCALL_SOCK_SHUTDOWN = 307,
    __EXT_SYSCALL_SOCK_GETSOCKOPT = 308,
    __EXT_SYSCALL_SOCK_SETSOCKOPT = 309,
    __EXT_SYSCALL_SOCK_RECVMSG = 310,
    __EXT_SYSCALL_SOCK_SENDMSG = 311,
    __EXT_SYSCALL_SOCK_GETIFADDRS = 312,
};

#pragma clang diagnostic pop
