//
// Created by kvxmmu on 5/3/20.
//

#ifndef OPENGROK2_DEF_HPP
#define OPENGROK2_DEF_HPP

#define BUFF_SIZE 8192
#define MAX_EPOLL_EVENTS 8192
#define NAME "[OpenGrok]"
#define SPACE ' '

#define HEADER_SIZE sizeof(uint8_t)

#include <cstdio>
#include <cstring>
#include <unistd.h>

#include <cstdlib>

#include <netinet/in.h>


sockaddr_in create_addr(unsigned short port, in_addr_t in_address = INADDR_ANY);
unsigned short get_listen_port(int fd);

int write_bytes(int fd, void *buffer, size_t bufflen);
int read_bytes(int fd, void *buffer, size_t bufflen, bool fatal = false);

void set_reuse(int fd);

#endif //OPENGROK2_DEF_HPP
