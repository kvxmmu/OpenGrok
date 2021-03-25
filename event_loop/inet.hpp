//
// Created by kvxmmu on 3/25/21.
//

#ifndef GROKLOOP_INET_HPP
#define GROKLOOP_INET_HPP

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string_view>
#include <ostream>

#include "bytebuffer.hpp"
#include "inet_errors.hpp"

static int read_bytes(int fd, char *buffer, size_t length) noexcept {
    int status = read(fd, buffer, length);

    if (status == -1) {
        perror("[GrokLoop:inet.hpp:read_bytes()] Error");
    }

    return status;
}

static int read_exact(int fd, char *buffer, size_t length) {
    int received = 0;

    while (received < length) {
        int bytes_read = read_bytes(fd, buffer+static_cast<size_t>(received),
                length-received);

        if (bytes_read <= 0) {
            throw Exceptions::NotEnoughBytes(length, received);
        }

        received += bytes_read;
    }

    return received;
}

static int write_bytes(int fd, char *buffer, size_t length) noexcept {
    int status = write(fd, buffer, length);

    if (status == -1) {
        perror("[GrokLoop:inet.hpp:write_bytes()] Error");
    }

    return status;
}

template <typename Integral>
static Integral read_integral(int fd) {
    char buffer[sizeof(Integral)];

    read_exact(fd, buffer, sizeof(Integral));

    return bytes_to_int<Integral>(buffer);
}

static bool is_disconnected(int fd) noexcept {
    uint8_t byte;

    int bytes_read = recv(fd, &byte, sizeof(byte), MSG_PEEK);

    return bytes_read <= 0;
}

class IPv4 {
public:
    std::string_view ip;
    sockaddr_in addr{};

    int sockfd;

    explicit IPv4(const sockaddr_in &_addr, int _sockfd) : sockfd(_sockfd) {
        this->ip = inet_ntoa(_addr.sin_addr);
        this->addr = _addr;
    }

    explicit IPv4(int fd) : sockfd(fd) {
        sockaddr_in caddr{};
        socklen_t len = sizeof(caddr);

        int result = getpeername(fd, reinterpret_cast<sockaddr *>(&caddr), &len);

        if (result == -1) {
            perror("IPv4()::IPv4(int fd) lookup error");

            this->ip = "0.0.0.0";
        } else {
            this->addr = caddr;
        }
    }

    [[nodiscard]] std::string_view str() const noexcept {
        return this->ip;
    }

    friend std::ostream &operator<<(std::ostream &stream, const IPv4 &ipv4) {
        stream << ipv4.str();

        return stream;
    }
};

#endif //GROKLOOP_INET_HPP
