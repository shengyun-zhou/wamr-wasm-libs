#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <ifaddrs.h>
#include <net/if.h>
#include <fcntl.h>

std::string get_addr_str(sockaddr* addr) {
    char buf[128];
    if (!addr)
        return "";
    if (addr->sa_family == AF_INET) {
        sockaddr_in* addr4 = (sockaddr_in*)addr;
        snprintf(buf, sizeof(buf), "%s:%hu", inet_ntoa(addr4->sin_addr), ntohs(addr4->sin_port));
    } else if (addr->sa_family == AF_INET6) {
        sockaddr_in6* addr6 = (sockaddr_in6*)addr;
        char ipv6Buf[64] = {0};
        inet_ntop(AF_INET6, addr6->sin6_addr.s6_addr, ipv6Buf, sizeof(ipv6Buf));
        snprintf(buf, sizeof(buf), "[%s]:%hu", ipv6Buf, ntohs(addr6->sin6_port));
    } else {
        snprintf(buf, sizeof(buf), "(unknown address family %d)\n", int(addr->sa_family));
    }
    return buf;
}

std::string get_addr_str(sockaddr_in* addr) {
    return get_addr_str((sockaddr*)addr);
}

void socket_set_blocking(int sockfd, bool blocking) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl(F_GETFL)");
        exit(1);
    }
    int fcntl_ret = 0;
    if (!blocking)
        fcntl_ret = fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    else
        fcntl_ret = fcntl(sockfd, F_SETFL, flags & (~O_NONBLOCK));
    if (fcntl_ret == -1) {
        perror("fcntl(F_SETFL)");
        exit(1);
    }
}

int main() {
    {
        struct ifaddrs* if_addrs = nullptr;
        if (getifaddrs(&if_addrs) == 0) {
            for (auto if_addr = if_addrs; if_addr; if_addr = if_addr->ifa_next) {
                printf("Net interface name: %s, running: %d, loopback: %d\n",
                       if_addr->ifa_name, (if_addr->ifa_flags & IFF_RUNNING) ? 1 : 0,
                       (if_addr->ifa_flags & IFF_LOOPBACK) ? 1 : 0);
                printf("IP address: %s, net mask: %s\n", get_addr_str(if_addr->ifa_addr).c_str(),
                       get_addr_str(if_addr->ifa_netmask).c_str());
            }
            freeifaddrs(if_addrs);
        } else {
            perror("getifaddr()");
            exit(1);
        }
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket()");
        exit(1);
    }
    int client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    socket_set_blocking(client_sockfd, false);
    int buf_size = 128 * 1024;
    if (setsockopt(client_sockfd, SOL_SOCKET, SO_RCVBUF, &buf_size, sizeof(buf_size)) != 0) {
        perror("setsockopt(SO_RCVBUF)");
        exit(1);
    }
    buf_size = 0;
    socklen_t optlen = sizeof(buf_size);
    if (getsockopt(client_sockfd, SOL_SOCKET, SO_RCVBUF, &buf_size, &optlen) != 0) {
        perror("getsockopt(SO_RCVBUF)");
        exit(1);
    }
    printf("SO_RCVBUF value: %d\n", buf_size);

    sockaddr_in bind_addr;
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    bind_addr.sin_port = htons(21188);
    if (bind(sockfd, (sockaddr*)&bind_addr, sizeof(bind_addr)) == -1) {
        perror("bind()");
        exit(1);
    }
    if (listen(sockfd, 64) == -1) {
        perror("listen()");
        exit(1);
    }
    memset(&bind_addr, 0, sizeof(bind_addr));
    socklen_t temp_addrlen = sizeof(bind_addr);
    if (getsockname(sockfd, (sockaddr*)&bind_addr, &temp_addrlen) == -1) {
        perror("getsockname()");
        exit(1);
    }
    printf("Local bind addr: %s\n", get_addr_str(&bind_addr).c_str());

    if (connect(client_sockfd, (sockaddr*)&bind_addr, sizeof(bind_addr)) == -1) {
        perror("connect()");
        if (errno != EINPROGRESS)
            exit(1);
    }
    const char send_data[] = "Hello, world!";
    int send_datalen = strlen(send_data);
    socket_set_blocking(client_sockfd, true);
    ssize_t sendret = send(client_sockfd, (char*)send_data, send_datalen, 0);
    if (sendret == -1) {
        perror("send()");
        exit(1);
    } else if (sendret != send_datalen) {
        printf("send() return %u, not all data has been sent\n", (unsigned int)sendret);
        exit(1);
    }

    sockaddr_in from_addr;
    temp_addrlen = sizeof(from_addr);
    int new_accept_sockfd = accept(sockfd, (sockaddr*)&from_addr, &temp_addrlen);
    if (new_accept_sockfd == -1) {
        perror("accept()");
        exit(1);
    }
    printf("Accepted new connection from %s\n", get_addr_str(&from_addr).c_str());
    memset(&from_addr, 0, sizeof(from_addr));
    temp_addrlen = sizeof(from_addr);
    if (getpeername(new_accept_sockfd, (sockaddr*)&from_addr, &temp_addrlen) == -1) {
        perror("getpeername()");
        exit(1);
    }
    printf("getpeername() of new accepted connection: %s\n", get_addr_str(&from_addr).c_str());
    char recvbuf[128] = {0};
    ssize_t recvret = recv(new_accept_sockfd, recvbuf, sizeof(recvbuf), 0);
    if (recvret == -1) {
        perror("recv()");
        exit(1);
    } else if (recvret != send_datalen) {
        printf("Not all data has been received\n");
        exit(1);
    }
    printf("Received %uB data: %s\n", (unsigned int)recvret, recvbuf);
    if (shutdown(client_sockfd, SHUT_WR) == -1) {
        perror("shutdown()");
        exit(1);
    }
    close(client_sockfd);
    close(new_accept_sockfd);
    close(sockfd);
    return 0;
}
