#include <errno.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wamr_ext.h>

int gethostname(char *name, size_t len) {
    // Ref: https://code.woboq.org/userspace/glibc/sysdeps/posix/gethostname.c.html
    struct utsname buf;
    size_t node_len;
    if (uname(&buf) != 0)
        return -1;
    node_len = strlen(buf.nodename) + 1;
    memcpy(name, buf.nodename, len < node_len ? len : node_len);
    if (node_len > len) {
        errno = ENAMETOOLONG;
        return -1;
    }
    return 0;
}
int sethostname(const char *name, size_t len) { errno = EPERM; return -1; }

pid_t getpid() {
    static pid_t g_pid = 0;
    if (g_pid == 0) {
        uint32_t temp_pid = 0;
        uint32_t buf_len = sizeof(temp_pid);
        wamr_ext_sysctl("sysinfo.pid", &temp_pid, &buf_len);
        g_pid = temp_pid;
    }
    return g_pid;
}

pid_t gettid() {
    _Thread_local static uint32_t tid = 0;
    if (tid == 0) {
        uint32_t buf_len = sizeof(tid);
        wamr_ext_sysctl("sysinfo.tid", &tid, &buf_len);
    }
    return tid;
}
