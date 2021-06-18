//
// Created by nero on 12.06.2021.
//

#ifndef GROKPP_SOCKETS_H
#define GROKPP_SOCKETS_H

#include "platform.h"

#ifdef _PLATFORM_WINDOWS
#include <windows.h>

typedef int socklen_t;
#else
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

#include <stdexcept>
#include <cstring>

class SysError : std::runtime_error {
public:
    SysError() : std::runtime_error(strerror(errno)) {

    }

public:
    const char *text() {
        return this->what();
    }
};


inline sock_t tcp_create(bool nonblocking = true) {
    int type = SOCK_STREAM;

#ifndef _PLATFORM_WINDOWS
    if (nonblocking) {
        type |= SOCK_NONBLOCK;
    }
#endif

    return socket(AF_INET, type, 0);
}

inline void tcp_bind(sock_t src, sockaddr_in &addr) {
    auto result = bind(src, reinterpret_cast<sockaddr *>(&addr),
                       sizeof(addr));

    if (result != 0) {
        throw SysError();
    }
}

inline void tcp_bind(sock_t src, uint32_t v4_addr,
                     uint16_t port) {
    sockaddr_in caddr{};
    caddr.sin_addr.s_addr = v4_addr;
    caddr.sin_port = htons(port);
    caddr.sin_family = AF_INET;

    tcp_bind(src, caddr);
}

inline void tcp_get_peer_name(sock_t src, sockaddr_in &out) {
    socklen_t len = sizeof(sockaddr_in);
    auto result = getpeername(src, reinterpret_cast<sockaddr *>(&out),
                              &len);
}

inline uint16_t tcp_get_listening_port(sock_t src) {
    sockaddr_in out{};
    socklen_t len = sizeof(sockaddr_in);

    if (getsockname(src, reinterpret_cast<sockaddr *>(&out), &len) != 0) {
        throw SysError();
    }

    return ntohs(out.sin_port);
}

inline auto tcp_accept(sock_t src, sockaddr_in &addr) {
    socklen_t len = sizeof(sockaddr_in);

    return accept(src, reinterpret_cast<sockaddr *>(&addr),
                  &len);
}

inline void tcp_connect(sock_t sock, sockaddr_in &addr) {
    auto result = connect(sock, reinterpret_cast<sockaddr *>(&addr),
                          sizeof(addr));
}

inline void tcp_connect(sock_t sock, uint32_t v4_addr,
                        uint16_t port) {
    sockaddr_in addr{};
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = v4_addr;
    addr.sin_family = AF_INET;

    tcp_connect(sock, addr);
}

inline void tcp_listen(sock_t src, int backlog = 4096) {
    auto result = listen(src, backlog);

    if (result != 0) {
        throw SysError();
    }
}

inline void tcp_set_reuse(sock_t src) {
    static int y = 1;
    auto status = setsockopt(src, SOL_SOCKET, SO_REUSEADDR,
#ifndef _PLATFORM_WINDOWS
                            &y
#else
                             reinterpret_cast<char *>(&y)
#endif
                             , sizeof(int));

    if (status != 0) {
        throw SysError();
    }
}

inline auto tcp_send(sock_t src, char *buffer,
                     size_t len, int flags = 0) {
    return send(src, buffer, len, flags);
}

inline auto tcp_recv(sock_t src, char *buffer,
                     size_t len, int flags = 0) {
    return recv(src, buffer, len,
                flags);
}

inline void tcp_close(sock_t src) {
#ifdef _PLATFORM_WINDOWS
    closesocket(src);
#else
    close(src);
#endif
}

#include <iostream>

inline bool tcp_is_port_available(uint16_t port) {
    auto test_socket = tcp_create(false);

    tcp_set_reuse(test_socket);

    sockaddr_in addr{};
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = AF_INET;

    try {
        tcp_bind(test_socket, addr);
    } catch (SysError &e) {
        tcp_close(test_socket);

        return false;
    }

    tcp_close(test_socket);

    return true;
}

inline bool tcp_is_connected(sock_t src) {
    char buf[1];
    auto len = tcp_recv(src, buf, 1,
                        MSG_PEEK);

    if (len == 0) {
        return false;
    } else if (len == -1) {
        return errno == EAGAIN || errno == EBADF || errno == ENOTCONN;
    }

    return true;
}

#endif //GROKPP_SOCKETS_H
