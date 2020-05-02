//
// Created by kvxmmu on 5/1/20.
//

#ifndef OPENGROK_UTILS_HPP
#define OPENGROK_UTILS_HPP

#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>

#include <grok/bin.hpp>

#include <cerrno>
#include <cstdio>

#define NAME "[OpenGrok]"
#define SPACE ' '

#define SEND_PORT static_cast<uint8_t>(1)
#define CONNECT static_cast<uint8_t>(2)
#define WRITE static_cast<uint8_t>(3)
#define CLOSE static_cast<uint8_t>(4)

#define HEADER_SIZE sizeof(uint8_t)

sockaddr_in create_addr(unsigned short port, in_addr_t addr = INADDR_ANY);
unsigned short get_listen_port(int fd);


int write_bytes(int fd, void *buffer, size_t bufflen);
int read_bytes(int fd, void *buffer, size_t bufflen, bool fatal=false);

template <typename T> int write_type(int fd, const T &data);
template <typename T> T read_type(int fd);

unsigned short retrieve_port(int fd);

void set_reuse(int fd);

void copy_port(unsigned short port, char *buff);
void copy_connect(int who, char *buff);
void copy_disconnect(int who, char *buff);
void copy_packet(int who, int length, char *packet, char *buff);

int connect_get_fd(sockaddr_in &caddr);


#endif //OPENGROK_UTILS_HPP
