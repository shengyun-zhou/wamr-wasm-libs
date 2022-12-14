#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <utime.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/statvfs.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "../internal/wamr_ext_syscall.h"

#define FS_EXT_MODULE "fs_ext"

int dup(int fildes) { errno = ENOSYS; return -1; }
int dup2(int oldfd, int newfd) { errno = ENOSYS; return -1; }

int chown(const char *path, uid_t owner, gid_t group) { errno = EPERM; return -1; }
int fchownat(int fd, const char *path, uid_t owner, gid_t group, int flag) { errno = EPERM; return -1; }
int fchown(int fildes, uid_t owner, gid_t group) { errno = EPERM; return -1; }
int lchown(const char *pathname, uid_t owner, gid_t group) { errno = EPERM; return -1; }

int chmod(const char *pathname, mode_t mode) { errno = EPERM; return -1; }
int fchmod(int fd, mode_t mode) { errno = EPERM; return -1; }
int fchmodat(int dirfd, const char *pathname, mode_t mode, int flags) { errno = EPERM; return -1; }

mode_t umask(mode_t mask) { return S_IXUSR | S_IXGRP | S_IXOTH; }

struct wamr_statvfs {
    uint32_t f_bsize;
    uint64_t f_blocks;
    uint64_t f_bfree;
    uint64_t f_bavail;
};

void __wamr_statvfs_to_statvfs(const struct wamr_statvfs *internal_buf, struct statvfs *out_buf) {
    memset(out_buf, 0, sizeof(*out_buf));
    out_buf->f_bsize = out_buf->f_frsize = internal_buf->f_bsize;
    out_buf->f_blocks = internal_buf->f_blocks;
    out_buf->f_bfree = internal_buf->f_bfree;
    out_buf->f_bavail = internal_buf->f_bavail;
}

int statvfs(const char *path, struct statvfs *buf) {
    int tempfd = open(path, O_RDONLY);
    if (tempfd == -1)
        return -1;
    int ret = fstatvfs(tempfd, buf);
    close(tempfd);
    return ret;
}

int fstatvfs(int fd, struct statvfs *buf) {
    struct wamr_statvfs internal_vfs_buf = {0};
    wamr_ext_syscall_arg argv[] = {
        {.u32 = fd},
        {.p = &internal_vfs_buf},
    };
    int err = __imported_wamr_ext_syscall(__EXT_SYSCALL_FD_STATVFS, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
    if (err != 0) {
        errno = err;
        return -1;
    }
    __wamr_statvfs_to_statvfs(&internal_vfs_buf, buf);
    return 0;
}

struct wamr_fcntl_struct_header {
    int32_t cmd;
    int32_t ret_value;
};

struct wamr_fcntl_generic {
    struct wamr_fcntl_struct_header fcntl_header;
};

struct wamr_fcntl_flock {
    struct wamr_fcntl_struct_header fcntl_header;
    int16_t l_type;
    int16_t l_whence;
    int64_t l_start;
    int64_t l_len;
};

#define DEFINE_FCNTL_STRUCT_VAR(struct_name, var_name, _cmd) \
    struct struct_name var_name = {0}; (var_name).fcntl_header.cmd = _cmd

int __wamr_ext_fcntl(int fd, int cmd, va_list args) {
    int ret;
    switch (cmd) {
        case F_GETLK:
        case F_SETLK:
        case F_SETLKW: {
            struct flock* p_flock = va_arg(args, struct flock*);
            DEFINE_FCNTL_STRUCT_VAR(wamr_fcntl_flock, flock_ctrl, cmd);
            flock_ctrl.l_type = p_flock->l_type;
            flock_ctrl.l_whence = p_flock->l_whence;
            flock_ctrl.l_start = p_flock->l_start;
            flock_ctrl.l_len = p_flock->l_len;
            wamr_ext_syscall_arg argv[] = {
                {.u32 = fd},
                {.p = &flock_ctrl},
            };
            int32_t err = __imported_wamr_ext_syscall(__EXT_SYSCALL_FD_EXT_FCNTL, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
            if (err != 0) {
                errno = err;
                ret = -1;
                break;
            }
            ret = flock_ctrl.fcntl_header.ret_value;
            if (cmd == F_GETLK) {
                p_flock->l_pid = 0;
                p_flock->l_type = flock_ctrl.l_type;
                p_flock->l_whence = flock_ctrl.l_whence;
                p_flock->l_start = flock_ctrl.l_start;
                p_flock->l_len = flock_ctrl.l_len;
            }
            break;
        }
        default:
            errno = EINVAL;
            ret = -1;
    }
    return ret;
}

int flock(int fd, int operation) {
    // Emulate as a call to fcntl().
    // Ref: glibc(https://code.woboq.org/userspace/glibc/sysdeps/posix/flock.c.html)
    struct flock lbuf;
    switch (operation & (~LOCK_NB)) {
        case LOCK_SH:
            lbuf.l_type = F_RDLCK;
            break;
        case LOCK_EX:
            lbuf.l_type = F_WRLCK;
            break;
        case LOCK_UN:
            lbuf.l_type = F_UNLCK;
            break;
        default:
            errno = EINVAL;
            return -1;
    }
    lbuf.l_whence = SEEK_SET;
    lbuf.l_start = lbuf.l_len = 0L;
    return fcntl(fd, (operation & LOCK_NB) ? F_SETLK : F_SETLKW, &lbuf);
}
