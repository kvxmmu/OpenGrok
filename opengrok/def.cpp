//
// Created by kvxmmu on 5/3/20.
//

#include "def.hpp"


sockaddr_in create_addr(unsigned short port, in_addr_t in_address) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = in_address;

    return addr;
}

unsigned short get_listen_port(int fd) {
    sockaddr_in addr{};
    socklen_t len = sizeof(addr);

    getsockname(fd, reinterpret_cast<sockaddr *>(&addr), &len);

    return ntohs(addr.sin_port);
}

int write_bytes(int fd, void *buffer, size_t bufflen) {
    int res = write(fd, buffer, bufflen);

    if (res < 0)
        perror("write()");

    return res;
}

int read_bytes(int fd, void *buffer, size_t bufflen, bool fatal) {
    int res = read(fd, buffer, bufflen);

    if (res < 0) {
        perror("read()");
        if (fatal)
            exit(0);
    }
    return res;
}

void set_reuse(int fd) {
    int y = 1;

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int));
}