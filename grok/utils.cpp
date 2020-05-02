//
// Created by kvxmmu on 5/1/20.
//

#include <cstdlib>
#include <iostream>
#include "utils.hpp"

sockaddr_in create_addr(unsigned short port, in_addr_t addr) {
    sockaddr_in caddr{};
    caddr.sin_port = htons(port);
    caddr.sin_family = AF_INET;
    caddr.sin_addr.s_addr = addr;
    return caddr;
}

unsigned short get_listen_port(int fd) {
    sockaddr_in addr{};
    socklen_t len = sizeof(addr);

    getsockname(fd, reinterpret_cast<sockaddr *>(&addr), &len);

    return ntohs(addr.sin_port);
}

int write_bytes(int fd, void *buffer, size_t bufflen) {
    int res = send(fd, buffer, bufflen, 0);
    if (res < 0) {
        std::cout << "write(): " << fd << std::endl;
        perror("write()");
    }
    return res;
}

int read_bytes(int fd, void *buffer, size_t bufflen, bool fatal) {
    int res = recv(fd, buffer, bufflen, 0);
    if (res < 0) {
        perror("recv()");
        if (fatal)
            exit(-errno);
    }
    return res;
}

template <typename T>
int write_type(int fd, const T &data) {
    return write_bytes(fd, &data, sizeof(T));
}

template <typename T>
T read_type(int fd) {
    T out{};

    read_bytes(fd, &out, sizeof(T));

    return out;
}

unsigned short retrieve_port(int fd) {
    read_type<uint8_t>(fd);
    auto data = read_type<unsigned short>(fd);

    return data;
}

void set_reuse(int fd) {
    int y = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int));
}

void copy_port(unsigned short port, char *buff) {
    BinaryBufferWriter writer(buff);
    writer.write(SEND_PORT);
    writer.write(port);
}

void copy_connect(int who, char *buff) {
    BinaryBufferWriter writer(buff);
    writer.write(CONNECT);
    writer.write(who);
}

void copy_disconnect(int who, char *buff) {
    BinaryBufferWriter writer(buff);
    writer.write(CLOSE);
    writer.write(who);
}

void copy_packet(int who, int length, char *packet, char *buff) {
    BinaryBufferWriter writer(buff);
    writer.write(WRITE);
    writer.write(who);
    writer.write(length);
    memcpy(buff+writer.offset, packet, length);
}

int connect_get_fd(sockaddr_in &caddr) {
    int sfd = socket(AF_INET, SOCK_STREAM, 0);

    int res = connect(sfd, reinterpret_cast<sockaddr *>(&caddr), sizeof(caddr));

    if (res < 0)
        perror("connect()");

    return sfd;
}
