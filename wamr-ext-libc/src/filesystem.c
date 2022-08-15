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
