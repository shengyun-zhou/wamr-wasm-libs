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
#include "../internal/priv_struct_header.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-attributes"

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
    wamr_wasi_struct_header _s_header;
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

int32_t __imported_fd_statvfs(int32_t fd, struct wamr_statvfs *buf) __attribute__((
    __import_module__(FS_EXT_MODULE),
    __import_name__("fd_statvfs")
));

int fstatvfs(int fd, struct statvfs *buf) {
    DEFINE_WAMR_WASI_STRUCT_VAR(wamr_statvfs, internal_vfs_buf, 0);
    int32_t err = __imported_fd_statvfs(fd, &internal_vfs_buf);
    if (err != 0) {
        errno = err;
        return -1;
    }
    __wamr_statvfs_to_statvfs(&internal_vfs_buf, buf);
    return 0;
}

#pragma clang diagnostic pop