#include <signal.h>
#include <errno.h>
#include <string.h>

int pthread_sigmask(int how, const sigset_t *set, sigset_t *oldset) {
    if (oldset)
        *oldset = 0;
    return 0;
}

int sigprocmask(int how, const sigset_t* set, sigset_t* oldset) {
    return pthread_sigmask(how, set, oldset);
}

int sigemptyset(sigset_t *set) {
    if (!set) {
        errno = EINVAL;
        return -1;
    }
    *set = 0;
    return 0;
}

int sigfillset(sigset_t *set) {
    if (!set) {
        errno = EINVAL;
        return -1;
    }
    memset(set, UINT8_MAX, sizeof(*set));
    return 0;
}

int sigaddset(sigset_t *set, int signum) {
    if (!set || signum <= 0) {
        errno = EINVAL;
        return -1;
    }
    *set |= 1U << (signum - 1);
    return 0;
}

int sigdelset(sigset_t *set, int signum) {
    if (!set || signum <= 0) {
        errno = EINVAL;
        return -1;
    }
    *set &= ~(1U << (signum - 1));
    return 0;
}

int sigismember(const sigset_t *set, int signum) {
    if (!set || signum <= 0) {
        errno = EINVAL;
        return -1;
    }
    uint32_t temp_mask = 1U << (signum - 1);
    if ((*set) & temp_mask)
        return 1;
    return 0;
}

