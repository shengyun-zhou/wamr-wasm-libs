#include <errno.h>
#include <netinet/in.h>
#include <stdint.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "../internal/wamr_ext_syscall.h"

int socket(int domain, int type, int protocol) {
    int32_t sockfd = -1;
    wamr_ext_syscall_arg argv[] = {
        {.u32 = domain},
        {.u32 = type},
        {.u32 = protocol},
        {.p = &sockfd},
    };
    int32_t err = __imported_wamr_ext_syscall(__EXT_SYSCALL_SOCK_OPEN, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
    if (err != 0) {
        errno = err;
        return -1;
    }
    return sockfd;
}

struct wamr_wasi_sockaddr_storage {
    _Alignas(8) uint16_t family;
    union {
        struct {
            uint8_t addr[4];
            uint16_t port;      // Network byte order
        } inet;
        struct {
            uint8_t addr[16];
            uint16_t port;      // Network byte order
            uint32_t flowinfo;
            uint32_t scope_id;
        } inet6;
        uint8_t __padding[56];
    } u_addr;
};

_Static_assert(sizeof(struct wamr_wasi_sockaddr_storage) == 64 && sizeof(struct wamr_wasi_sockaddr_storage) >= sizeof(struct sockaddr_storage) + 16,
        "sizeof(wamr_wasi_sockaddr_storage) must be 64 bytes");

bool __sockaddr_to_wamr_wasi_sockaddr_storage(const struct sockaddr *addr, socklen_t addrlen, struct wamr_wasi_sockaddr_storage* wasi_sockaddr) {
    if (!addr) {
        errno = EINVAL;
        return false;
    }
    wasi_sockaddr->family = addr->sa_family;
    if (wasi_sockaddr->family == AF_INET) {
        if (addrlen < sizeof(struct sockaddr_in)) {
            errno = EINVAL;
            return false;
        }
        const struct sockaddr_in* temp_addr4 = (struct sockaddr_in*)addr;
        memcpy(wasi_sockaddr->u_addr.inet.addr, &temp_addr4->sin_addr.s_addr, 4);
        wasi_sockaddr->u_addr.inet.port = temp_addr4->sin_port;
        return true;
    } else if (wasi_sockaddr->family == AF_INET6) {
        if (addrlen < sizeof(struct sockaddr_in6)) {
            errno = EINVAL;
            return false;
        }
        const struct sockaddr_in6* temp_addr6 = (struct sockaddr_in6*)addr;
        memcpy(wasi_sockaddr->u_addr.inet6.addr, temp_addr6->sin6_addr.s6_addr, 16);
        wasi_sockaddr->u_addr.inet6.port = temp_addr6->sin6_port;
        wasi_sockaddr->u_addr.inet6.flowinfo = temp_addr6->sin6_flowinfo;
        wasi_sockaddr->u_addr.inet6.scope_id = temp_addr6->sin6_scope_id;
    } else {
        errno = EINVAL;
        return false;
    }
    return true;
}

void __wamr_wasi_sockaddr_storage_to_sockaddr(const struct wamr_wasi_sockaddr_storage* wasi_sockaddr, struct sockaddr* addr, socklen_t* addrlen) {
    if (*addrlen == 0)
        return;
    struct sockaddr_storage temp_sockaddr;
    if (wasi_sockaddr->family == AF_INET) {
        struct sockaddr_in* sock_addr4 = (struct sockaddr_in*)&temp_sockaddr;
        memcpy(&sock_addr4->sin_addr.s_addr, wasi_sockaddr->u_addr.inet.addr, 4);
        sock_addr4->sin_port = wasi_sockaddr->u_addr.inet.port;
        if (*addrlen > sizeof(struct sockaddr_in))
            *addrlen = sizeof(struct sockaddr_in);
    } else if (wasi_sockaddr->family == AF_INET6) {
        struct sockaddr_in6* sock_addr6 = (struct sockaddr_in6*)&temp_sockaddr;
        memcpy(&sock_addr6->sin6_addr.s6_addr, wasi_sockaddr->u_addr.inet6.addr, 16);
        sock_addr6->sin6_port = wasi_sockaddr->u_addr.inet6.port;
        sock_addr6->sin6_flowinfo = wasi_sockaddr->u_addr.inet6.flowinfo;
        sock_addr6->sin6_scope_id = wasi_sockaddr->u_addr.inet6.scope_id;
        if (*addrlen > sizeof(struct sockaddr_in6))
            *addrlen = sizeof(struct sockaddr_in6);
    } else {
        return;
    }
    temp_sockaddr.ss_family = wasi_sockaddr->family;
    memcpy(addr, &temp_sockaddr, *addrlen);
}

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    struct wamr_wasi_sockaddr_storage wasi_sockaddr = {0};
    if (!__sockaddr_to_wamr_wasi_sockaddr_storage(addr, addrlen, &wasi_sockaddr))
        return -1;
    wamr_ext_syscall_arg argv[] = {
        {.u32 = sockfd},
        {.p = &wasi_sockaddr},
    };
    int32_t err = __imported_wamr_ext_syscall(__EXT_SYSCALL_SOCK_BIND, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
    if (err != 0) {
        errno = err;
        return -1;
    }
    return 0;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    struct wamr_wasi_sockaddr_storage wasi_sockaddr = {0};
    if (!__sockaddr_to_wamr_wasi_sockaddr_storage(addr, addrlen, &wasi_sockaddr))
        return -1;
    wamr_ext_syscall_arg argv[] = {
        {.u32 = sockfd},
        {.p = &wasi_sockaddr},
    };
    int32_t err = __imported_wamr_ext_syscall(__EXT_SYSCALL_SOCK_CONNECT, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
    if (err != 0) {
        errno = err;
        return -1;
    }
    return 0;
}

int listen(int sockfd, int backlog) {
    wamr_ext_syscall_arg argv[] = {
        {.u32 = sockfd},
        {.u32 = backlog},
    };
    int32_t err = __imported_wamr_ext_syscall(__EXT_SYSCALL_SOCK_LISTEN, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
    if (err != 0) {
        errno = err;
        return -1;
    }
    return 0;
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    if (addr && !addrlen) {
        errno = EINVAL;
        return -1;
    }
    int32_t new_sockfd = -1;
    struct wamr_wasi_sockaddr_storage wasi_sockaddr = {0};
    wamr_ext_syscall_arg argv[] = {
        {.u32 = sockfd},
        {.p = &new_sockfd},
        {.p = &wasi_sockaddr},
    };
    int32_t err = __imported_wamr_ext_syscall(__EXT_SYSCALL_SOCK_ACCEPT, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
    if (err != 0) {
        errno = err;
        return -1;
    }
    if (addr)
        __wamr_wasi_sockaddr_storage_to_sockaddr(&wasi_sockaddr, addr, addrlen);
    return new_sockfd;
}

int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    if (!addr || !addrlen) {
        errno = EINVAL;
        return -1;
    }
    struct wamr_wasi_sockaddr_storage wasi_sockaddr = {0};
    wamr_ext_syscall_arg argv[] = {
        {.u32 = sockfd},
        {.p = &wasi_sockaddr},
    };
    int32_t err = __imported_wamr_ext_syscall(__EXT_SYSCALL_SOCK_GETSOCKNAME, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
    if (err != 0) {
        errno = err;
        return -1;
    }
    __wamr_wasi_sockaddr_storage_to_sockaddr(&wasi_sockaddr, addr, addrlen);
    return 0;
}

int getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    if (!addr || !addrlen) {
        errno = EINVAL;
        return -1;
    }
    struct wamr_wasi_sockaddr_storage wasi_sockaddr = {0};
    wamr_ext_syscall_arg argv[] = {
        {.u32 = sockfd},
        {.p = &wasi_sockaddr},
    };
    int32_t err = __imported_wamr_ext_syscall(__EXT_SYSCALL_SOCK_GETPEERNAME, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
    if (err != 0) {
        errno = err;
        return -1;
    }
    __wamr_wasi_sockaddr_storage_to_sockaddr(&wasi_sockaddr, addr, addrlen);
    return 0;
}

int shutdown(int sockfd, int how) {
    wamr_ext_syscall_arg argv[] = {
        {.u32 = sockfd},
        {.u32 = how},
    };
    int32_t err = __imported_wamr_ext_syscall(__EXT_SYSCALL_SOCK_SHUTDOWN, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
    if (err != 0) {
        errno = err;
        return -1;
    }
    return 0;
}

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen) {
    if (!optval || !optlen || *optlen <= 0) {
        errno = EINVAL;
        return -1;
    }
    uint32_t wasi_optlen = *optlen;
    wamr_ext_syscall_arg argv[] = {
        {.u32 = sockfd},
        {.u32 = level},
        {.u32 = optname},
        {.p = optval},
        {.p = &wasi_optlen},
    };
    int32_t ret = __imported_wamr_ext_syscall(__EXT_SYSCALL_SOCK_GETSOCKOPT, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
    *optlen = wasi_optlen;
    if (ret != 0) {
        errno = ret;
        return -1;
    }
    return 0;
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
    if (!optval || optlen <= 0) {
        errno = EINVAL;
        return -1;
    }
    wamr_ext_syscall_arg argv[] = {
        {.u32 = sockfd},
        {.u32 = level},
        {.u32 = optname},
        {.p = (void*)optval},
        {.u32 = optlen},
    };
    int32_t ret = __imported_wamr_ext_syscall(__EXT_SYSCALL_SOCK_SETSOCKOPT, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
    if (ret != 0) {
        errno = ret;
        return -1;
    }
    return 0;
}

ssize_t recvfrom(int sock, void *restrict buffer, size_t length, int flags, struct sockaddr *restrict address,
                 socklen_t *restrict address_len) {
    if (address && !address_len)
        return -1;
    struct msghdr msg;
    memset(&msg, 0, sizeof(msg));
    struct iovec tempiovec;
    tempiovec.iov_base = buffer;
    tempiovec.iov_len = length;
    msg.msg_iov = &tempiovec;
    msg.msg_iovlen = 1;
    msg.msg_name = address;
    msg.msg_namelen = address_len ? *address_len : 0;
    ssize_t ret = recvmsg(sock, &msg, flags);
    if (address_len)
        *address_len = msg.msg_namelen;
    return ret;
}

struct wamr_wasi_msghdr {
    struct wamr_wasi_sockaddr_storage addr;
    uint32_t input_flags;
    uint32_t ret_flags;
    uint64_t ret_data_size;
};

ssize_t recvmsg(int sockfd, struct msghdr *message, int flags) {
    if (!message || !message->msg_iov || message->msg_iovlen <= 0) {
        errno = EINVAL;
        return -1;
    }
    struct wamr_wasi_msghdr wamr_msghdr = {0};
    wamr_msghdr.addr.family = AF_UNSPEC;
    wamr_msghdr.input_flags = flags;
    wamr_ext_syscall_arg argv[] = {
        {.u32 = sockfd},
        {.p = &wamr_msghdr},
        {.p = message->msg_iov},
        {.u32 = message->msg_iovlen}
    };
    int32_t err = __imported_wamr_ext_syscall(__EXT_SYSCALL_SOCK_RECVMSG, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
    if (err != 0) {
        errno = err;
        return -1;
    }
    if (message->msg_name)
        __wamr_wasi_sockaddr_storage_to_sockaddr(&wamr_msghdr.addr, (struct sockaddr*)message->msg_name, &message->msg_namelen);
    message->msg_flags = wamr_msghdr.ret_flags;
    return wamr_msghdr.ret_data_size;
}

ssize_t sendto(int sock, const void *message, size_t length, int flags, const struct sockaddr *dest_addr,
               socklen_t dest_len) {
    struct msghdr msg;
    memset(&msg, 0, sizeof(msg));
    struct iovec tempiovec;
    tempiovec.iov_base = (void*)message;
    tempiovec.iov_len = length;
    msg.msg_iov = &tempiovec;
    msg.msg_iovlen = 1;
    msg.msg_name = (void*)dest_addr;
    msg.msg_namelen = dest_len;
    return sendmsg(sock, &msg, flags);
}

ssize_t sendmsg(int sockfd, const struct msghdr *message, int flags) {
    if (!message || !message->msg_iov || message->msg_iovlen <= 0) {
        errno = EINVAL;
        return -1;
    }
    struct wamr_wasi_msghdr wamr_msghdr = {0};
    wamr_msghdr.addr.family = AF_UNSPEC;
    if (message->msg_name && !__sockaddr_to_wamr_wasi_sockaddr_storage((struct sockaddr*)message->msg_name, message->msg_namelen, &wamr_msghdr.addr))
        return -1;
    wamr_msghdr.input_flags = flags;
    wamr_ext_syscall_arg argv[] = {
        {.u32 = sockfd},
        {.p = &wamr_msghdr},
        {.p = message->msg_iov},
        {.u32 = message->msg_iovlen}
    };
    int32_t err = __imported_wamr_ext_syscall(__EXT_SYSCALL_SOCK_SENDMSG, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
    if (err != 0) {
        errno = err;
        return -1;
    }
    return wamr_msghdr.ret_data_size;
}

#define WAMR_IF_NAME_MAX_LEN 32
_Static_assert(WAMR_IF_NAME_MAX_LEN >= IF_NAMESIZE, "WAMR_IF_NAME_MAX_LEN must >= IF_NAMESIZE");

struct _wamr_ifaddr_buf {
    struct ifaddrs ifa_hdr;
    char ifa_name[WAMR_IF_NAME_MAX_LEN];
    struct sockaddr_storage ifa_addr;
    struct sockaddr_storage ifa_netmask;
    struct sockaddr_storage ifu_broadaddr;
    struct ifaddrs_extdata ifa_extdata;
};

struct wamr_wasi_ifaddr {
    char ifa_name[WAMR_IF_NAME_MAX_LEN];
    uint32_t ifa_flags;
    struct wamr_wasi_sockaddr_storage ifa_addr;
    struct wamr_wasi_sockaddr_storage ifa_netmask;
    struct wamr_wasi_sockaddr_storage ifu_broadaddr;
    uint8_t ifa_hwaddr[6];
    uint32_t ifa_ifindex;
};

struct wamr_wasi_ifaddrs_req {
    void*(*func_malloc)(uint32_t);
    struct wamr_wasi_ifaddr* ret_ifaddrs;
    uint32_t ret_ifaddr_cnt;
};

void* __getifaddrs_malloc(uint32_t s) { return malloc(s); }

int32_t __wamr_sock_getifaddrs(struct wamr_wasi_ifaddrs_req* req) {
    req->func_malloc = __getifaddrs_malloc;
    wamr_ext_syscall_arg argv[] = {
        {.p = req},
    };
    return __imported_wamr_ext_syscall(__EXT_SYSCALL_SOCK_GETIFADDRS, sizeof(argv) / sizeof(wamr_ext_syscall_arg), argv);
}

int getifaddrs(struct ifaddrs **ifap) {
    struct wamr_wasi_ifaddrs_req req = {0};
    int32_t err = __wamr_sock_getifaddrs(&req);
    if (err != 0) {
        errno = err;
        return -1;
    }
    if (req.ret_ifaddr_cnt == 0) {
        *ifap = NULL;
        return 0;
    }
    struct _wamr_ifaddr_buf* ifaddrs_buf = malloc(sizeof(struct _wamr_ifaddr_buf) * req.ret_ifaddr_cnt);
    for (uint32_t i = 0; i < req.ret_ifaddr_cnt; i++) {
        memcpy(ifaddrs_buf[i].ifa_name, req.ret_ifaddrs[i].ifa_name, sizeof(ifaddrs_buf[i].ifa_name));
        socklen_t _templen = sizeof(struct sockaddr_storage);
        ifaddrs_buf[i].ifa_addr.ss_family = AF_UNSPEC;
        __wamr_wasi_sockaddr_storage_to_sockaddr(&req.ret_ifaddrs[i].ifa_addr, (struct sockaddr*)&ifaddrs_buf[i].ifa_addr, &_templen);

        _templen = sizeof(struct sockaddr_storage);
        ifaddrs_buf[i].ifa_netmask.ss_family = AF_UNSPEC;
        __wamr_wasi_sockaddr_storage_to_sockaddr(&req.ret_ifaddrs[i].ifa_netmask, (struct sockaddr*)&ifaddrs_buf[i].ifa_netmask, &_templen);

        _templen = sizeof(struct sockaddr_storage);
        ifaddrs_buf[i].ifu_broadaddr.ss_family = AF_UNSPEC;
        __wamr_wasi_sockaddr_storage_to_sockaddr(&req.ret_ifaddrs[i].ifu_broadaddr, (struct sockaddr*)&ifaddrs_buf[i].ifu_broadaddr, &_templen);

        memcpy(ifaddrs_buf[i].ifa_extdata.ifa_hwaddr, req.ret_ifaddrs[i].ifa_hwaddr, 6);
        ifaddrs_buf[i].ifa_extdata.ifa_ifindex = req.ret_ifaddrs[i].ifa_ifindex;

        ifaddrs_buf[i].ifa_hdr.ifa_name = ifaddrs_buf[i].ifa_name;
        ifaddrs_buf[i].ifa_hdr.ifa_flags = req.ret_ifaddrs[i].ifa_flags;
        ifaddrs_buf[i].ifa_hdr.ifa_addr = ifaddrs_buf[i].ifa_addr.ss_family == AF_UNSPEC ? NULL : (struct sockaddr*)&ifaddrs_buf[i].ifa_addr;
        ifaddrs_buf[i].ifa_hdr.ifa_netmask = ifaddrs_buf[i].ifa_netmask.ss_family == AF_UNSPEC ? NULL : (struct sockaddr*)&ifaddrs_buf[i].ifa_netmask;
        ifaddrs_buf[i].ifa_hdr.ifa_broadaddr = ifaddrs_buf[i].ifu_broadaddr.ss_family == AF_UNSPEC ? NULL : (struct sockaddr*)&ifaddrs_buf[i].ifu_broadaddr;
        ifaddrs_buf[i].ifa_hdr.ifa_data = &ifaddrs_buf[i].ifa_extdata;
        ifaddrs_buf[i].ifa_hdr.ifa_next = (i + 1 == req.ret_ifaddr_cnt) ? NULL : &ifaddrs_buf[i + 1].ifa_hdr;
    }
    *ifap = &ifaddrs_buf[0].ifa_hdr;
    free(req.ret_ifaddrs);
    return 0;
}

void freeifaddrs(struct ifaddrs *ifa) {
    if (!ifa)
        return;
    free(ifa);
}

unsigned int if_nametoindex(const char *ifname) {
    struct wamr_wasi_ifaddrs_req req = {0};
    int32_t err = __wamr_sock_getifaddrs(&req);
    if (err != 0) {
        errno = err;
        return 0;
    }
    if (req.ret_ifaddr_cnt == 0) {
        errno = ENODEV;
        return 0;
    }
    unsigned int idx = 0;
    for (uint32_t i = 0; i < req.ret_ifaddr_cnt; i++) {
        if (strcmp(req.ret_ifaddrs[i].ifa_name, ifname) == 0) {
            idx = req.ret_ifaddrs[i].ifa_ifindex;
            break;
        }
    }
    free(req.ret_ifaddrs);
    if (idx == 0)
        errno = ENODEV;
    return idx;
}

char *if_indextoname(unsigned int ifindex, char *ifname) {
    struct wamr_wasi_ifaddrs_req req = {0};
    int32_t err = __wamr_sock_getifaddrs(&req);
    if (err != 0) {
        errno = err;
        return NULL;
    }
    if (req.ret_ifaddr_cnt == 0) {
        errno = ENXIO;
        return NULL;
    }
    char* ret = NULL;
    for (uint32_t i = 0; i < req.ret_ifaddr_cnt; i++) {
        if (ifindex == req.ret_ifaddrs[i].ifa_ifindex) {
            snprintf(ifname, IF_NAMESIZE, "%s", req.ret_ifaddrs[i].ifa_name);
            ret = ifname;
            break;
        }
    }
    free(req.ret_ifaddrs);
    if (!ret)
        errno = ENXIO;
    return ret;
}

struct if_nameindex *if_nameindex(void) {
    struct wamr_wasi_ifaddrs_req req = {0};
    int32_t err = __wamr_sock_getifaddrs(&req);
    if (err != 0) {
        errno = err;
        return NULL;
    }
    struct if_nameindex* ret_ni = malloc(sizeof(struct if_nameindex) * (req.ret_ifaddr_cnt + 1));
    uint32_t ret_cnt = 0;
    for (uint32_t i = 0; i < req.ret_ifaddr_cnt; i++) {
        bool idx_exist = false;
        for (uint32_t j = 0; j < ret_cnt; j++) {
            if (req.ret_ifaddrs[i].ifa_ifindex == ret_ni[j].if_index) {
                idx_exist = true;
                break;
            }
        }
        if (!idx_exist) {
            ret_ni[ret_cnt].if_index = req.ret_ifaddrs[i].ifa_ifindex;
            ret_ni[ret_cnt].if_name = strdup(req.ret_ifaddrs[i].ifa_name);
            ret_cnt++;
        }
    }
    free(req.ret_ifaddrs);
    ret_ni[ret_cnt].if_index = 0;
    ret_ni[ret_cnt].if_name = NULL;
    return ret_ni;
}

void if_freenameindex(struct if_nameindex *ptr) {
    if (!ptr)
        return;
    for (struct if_nameindex* p = ptr; !(p->if_index == 0 && p->if_name == NULL); p++)
        free(p->if_name);
    free(ptr);
}

void __socket_set_blocking(int sockfd, bool blocking) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1)
        return;
    if (!blocking)
        fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    else
        fcntl(sockfd, F_SETFL, flags & (~O_NONBLOCK));
}

int socketpair(int domain, int type, int protocol, int socket_vector[2]) {
    int lsock = socket(domain, type, protocol);
    int csock = socket(domain, type, protocol);
    do {
        if (lsock == -1 || csock == -1)
            break;
        {
            uint32_t _random_num = 0;
            getentropy(&_random_num, sizeof(_random_num));
            if (_random_num != 0)
                srandom(_random_num);
        }
        struct sockaddr_storage laddr = {0};
        laddr.ss_family = domain;
        socklen_t laddr_len = 0;
        if (laddr.ss_family == AF_INET) {
            struct sockaddr_in* laddr4 = (struct sockaddr_in*)&laddr;
            laddr4->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            laddr_len = sizeof(struct sockaddr_in);
        } else if (laddr.ss_family == AF_INET6) {
            struct sockaddr_in6* laddr6 = (struct sockaddr_in6*)&laddr;
            laddr6->sin6_addr.s6_addr[15] = 1;
            laddr_len = sizeof(struct sockaddr_in6);
        }
        {
            int r;
            for (r = 0; r < 100; r++) {
                uint16_t random_port = (random() % 16284U) + 49152U;
                if (laddr.ss_family == AF_INET)
                    ((struct sockaddr_in*)&laddr)->sin_port = htons(random_port);
                else if (laddr.ss_family == AF_INET6)
                    ((struct sockaddr_in6*)&laddr)->sin6_port = htons(random_port);
                if (bind(lsock, (struct sockaddr*)&laddr, laddr_len) == 0)
                    break;
            }
            if (r >= 100)
                break;
        }
        if ((type & SOCK_STREAM) == SOCK_STREAM) {
            if (listen(lsock, 4) != 0)
                break;
        }
        __socket_set_blocking(csock, false);
        int connret = connect(csock, (struct sockaddr*)&laddr, laddr_len);
        if (connret != 0) {
            int err = errno;
            if (err != EAGAIN && err != EWOULDBLOCK && err != EINPROGRESS)
                break;
        }
        if ((type & SOCK_STREAM) == SOCK_STREAM) {
            int ssock = accept(lsock, NULL, NULL);
            if (ssock == -1)
                break;
            close(lsock);
            __socket_set_blocking(ssock, (type & SOCK_NONBLOCK) != SOCK_NONBLOCK);
            socket_vector[0] = ssock; socket_vector[1] = csock;
        } else {
            socket_vector[0] = lsock; socket_vector[1] = csock;
        }
        if ((type & SOCK_NONBLOCK) != SOCK_NONBLOCK)
            __socket_set_blocking(csock, true);
        return 0;
    } while (false);
    if (lsock != -1)
        close(lsock);
    if (csock != -1)
        close(csock);
    return -1;
}
