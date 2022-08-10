#include <unistd.h>
#include <errno.h>

uid_t getuid() { return 0; }
uid_t geteuid() { return 0; }
uid_t getgid() { return 0; }
gid_t getegid() { return 0; }

int setuid(uid_t uid) { errno = EPERM; return -1; }
int seteuid(uid_t euid) { errno = EPERM; return -1; }
int setgid(gid_t gid) { errno = EPERM; return -1; }
int setegid(gid_t egid) { errno = EPERM; return -1; }
