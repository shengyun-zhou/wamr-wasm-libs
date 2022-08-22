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
#include "../internal/priv_struct_header.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-attributes"

#define SOCKET_EXT_MODULE "socket_ext"

int32_t __imported_sock_open(int32_t, int32_t, int32_t, int32_t*) __attribute__((
    __import_module__(SOCKET_EXT_MODULE),
    __import_name__("sock_open")
));

int socket(int domain, int type, int protocol) {
    int32_t sockfd = -1;
    int32_t err = __imported_sock_open(domain, type, protocol, &sockfd);
    if (err != 0) {
        errno = err;
        return -1;
    }
    return sockfd;
}

struct wamr_wasi_sockaddr_storage {
    _Alignas(8) struct wamr_wasi_struct_header _s_header;
    uint16_t family;
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

int32_t __imported_sock_bind(int32_t, const struct wamr_wasi_sockaddr_storage*) __attribute__((
    __import_module__(SOCKET_EXT_MODULE),
    __import_name__("sock_bind")
));

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    DEFINE_WAMR_WASI_STRUCT_VAR(wamr_wasi_sockaddr_storage, wasi_sockaddr, 0);
    if (!__sockaddr_to_wamr_wasi_sockaddr_storage(addr, addrlen, &wasi_sockaddr))
        return -1;
    int32_t err = __imported_sock_bind(sockfd, &wasi_sockaddr);
    if (err != 0) {
        errno = err;
        return -1;
    }
    return 0;
}

int32_t __imported_sock_connect(int32_t sockfd, const struct wamr_wasi_sockaddr_storage*) __attribute__((
    __import_module__(SOCKET_EXT_MODULE),
    __import_name__("sock_connect")
));

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    DEFINE_WAMR_WASI_STRUCT_VAR(wamr_wasi_sockaddr_storage, wasi_sockaddr, 0);
    if (!__sockaddr_to_wamr_wasi_sockaddr_storage(addr, addrlen, &wasi_sockaddr))
        return -1;
    int32_t err = __imported_sock_connect(sockfd, &wasi_sockaddr);
    if (err != 0) {
        errno = err;
        return -1;
    }
    return 0;
}

int32_t __imported_sock_listen(int32_t sockfd, int32_t backlog) __attribute__((
    __import_module__(SOCKET_EXT_MODULE),
    __import_name__("sock_listen")
));

int listen(int sockfd, int backlog) {
    int32_t err = __imported_sock_listen(sockfd, backlog);
    if (err != 0) {
        errno = err;
        return -1;
    }
    return 0;
}

int32_t __imported_sock_accept(int32_t, int32_t*, struct wamr_wasi_sockaddr_storage*) __attribute__((
    __import_module__(SOCKET_EXT_MODULE),
    __import_name__("sock_accept")
));

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    if (addr && !addrlen) {
        errno = EINVAL;
        return -1;
    }
    int32_t new_sockfd = -1;
    DEFINE_WAMR_WASI_STRUCT_VAR(wamr_wasi_sockaddr_storage, wasi_sockaddr, 0);
    int32_t err = __imported_sock_accept(sockfd, &new_sockfd, &wasi_sockaddr);
    if (err != 0) {
        errno = err;
        return -1;
    }
    if (addr)
        __wamr_wasi_sockaddr_storage_to_sockaddr(&wasi_sockaddr, addr, addrlen);
    return new_sockfd;
}

int32_t __imported_sock_getsockname(int32_t, struct wamr_wasi_sockaddr_storage*) __attribute__((
    __import_module__(SOCKET_EXT_MODULE),
    __import_name__("sock_getsockname")
));

int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    if (!addr || !addrlen) {
        errno = EINVAL;
        return -1;
    }
    DEFINE_WAMR_WASI_STRUCT_VAR(wamr_wasi_sockaddr_storage, wasi_sockaddr, 0);
    int32_t err = __imported_sock_getsockname(sockfd, &wasi_sockaddr);
    if (err != 0) {
        errno = err;
        return -1;
    }
    __wamr_wasi_sockaddr_storage_to_sockaddr(&wasi_sockaddr, addr, addrlen);
    return 0;
}

int32_t __imported_sock_getpeername(int32_t, struct wamr_wasi_sockaddr_storage*) __attribute__((
    __import_module__(SOCKET_EXT_MODULE),
    __import_name__("sock_getpeername")
));

int getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    if (!addr || !addrlen) {
        errno = EINVAL;
        return -1;
    }
    DEFINE_WAMR_WASI_STRUCT_VAR(wamr_wasi_sockaddr_storage, wasi_sockaddr, 0);
    int32_t err = __imported_sock_getpeername(sockfd, &wasi_sockaddr);
    if (err != 0) {
        errno = err;
        return -1;
    }
    __wamr_wasi_sockaddr_storage_to_sockaddr(&wasi_sockaddr, addr, addrlen);
    return 0;
}

int32_t __imported_sock_shutdown(int32_t, int32_t) __attribute__((
    __import_module__(SOCKET_EXT_MODULE),
    __import_name__("sock_shutdown")
));

int shutdown(int sock, int how) {
    int32_t err = __imported_sock_shutdown(sock, how);
    if (err != 0) {
        errno = err;
        return -1;
    }
    return 0;
}

int32_t __imported_sock_getopt(int32_t, int32_t, int32_t, void*, uint32_t*) __attribute__((
    __import_module__(SOCKET_EXT_MODULE),
    __import_name__("sock_getopt")
));

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen) {
    if (!optval || !optlen || *optlen <= 0) {
        errno = EINVAL;
        return -1;
    }
    uint32_t wasi_optlen = *optlen;
    int32_t ret = __imported_sock_getopt(sockfd, level, optname, optval, &wasi_optlen);
    *optlen = wasi_optlen;
    if (ret != 0) {
        errno = ret;
        return -1;
    }
    return 0;
}

int32_t __imported_sock_setopt(int32_t, int32_t, int32_t, const void*, uint32_t) __attribute__((
    __import_module__(SOCKET_EXT_MODULE),
    __import_name__("sock_setopt")
));

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
    if (!optval || optlen <= 0) {
        errno = EINVAL;
        return -1;
    }
    int32_t ret = __imported_sock_setopt(sockfd, level, optname, optval, optlen);
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
    struct wamr_wasi_struct_header _s_header;
    struct wamr_wasi_sockaddr_storage addr;
    uint32_t input_flags;
    uint32_t ret_flags;
    uint64_t ret_data_size;
};

int32_t __imported_sock_recvmsg(int32_t, struct wamr_wasi_msghdr*, struct iovec*, uint32_t) __attribute__((
    __import_module__(SOCKET_EXT_MODULE),
    __import_name__("sock_recvmsg")
));

ssize_t recvmsg(int sock, struct msghdr *message, int flags) {
    if (!message->msg_iov || message->msg_iovlen <= 0) {
        errno = EINVAL;
        return -1;
    }
    DEFINE_WAMR_WASI_STRUCT_VAR(wamr_wasi_msghdr, wamr_msghdr, 0);
    INIT_WAMR_WASI_STRUCT_VAR(wamr_msghdr.addr, 0);
    wamr_msghdr.addr.family = AF_UNSPEC;
    wamr_msghdr.input_flags = flags;
    int32_t err = __imported_sock_recvmsg(sock, &wamr_msghdr, message->msg_iov, message->msg_iovlen);
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

int32_t __imported_sock_sendmsg(int32_t, struct wamr_wasi_msghdr*, struct iovec*, uint32_t) __attribute__((
    __import_module__(SOCKET_EXT_MODULE),
    __import_name__("sock_sendmsg")
));

ssize_t sendmsg(int sock, const struct msghdr *message, int flags) {
    if (!message->msg_iov || message->msg_iovlen <= 0) {
        errno = EINVAL;
        return -1;
    }
    DEFINE_WAMR_WASI_STRUCT_VAR(wamr_wasi_msghdr, wamr_msghdr, 0);
    INIT_WAMR_WASI_STRUCT_VAR(wamr_msghdr.addr, 0);
    wamr_msghdr.addr.family = AF_UNSPEC;
    if (message->msg_name && !__sockaddr_to_wamr_wasi_sockaddr_storage((struct sockaddr*)message->msg_name, message->msg_namelen, &wamr_msghdr.addr))
        return -1;
    wamr_msghdr.input_flags = flags;
    int32_t err = __imported_sock_sendmsg(sock, &wamr_msghdr, message->msg_iov, message->msg_iovlen);
    if (err != 0) {
        errno = err;
        return -1;
    }
    return wamr_msghdr.ret_data_size;
}

#pragma clang diagnostic pop
