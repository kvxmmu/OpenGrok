//
// Created by kvxmmu on 3/27/21.
//

#ifndef CAMELEO_NET_HELPERS_H
#define CAMELEO_NET_HELPERS_H

#include <cstdio>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>

#include <stdexcept>

namespace Cameleo::Net {
    // MSG_NOSIGNAL is used to suppress SIGPIPE error

    static int read_bytes(int fd, char *buffer, size_t length) {
        int bytes_read = recv(fd, buffer, length, MSG_NOSIGNAL);

        if (bytes_read == -1) {
            perror("read_bytes()");
        }

        return bytes_read;
    }

    static int write_exact(int fd, char *buffer, size_t length) {
        size_t written = 0;

        while (written < length) {
            int bytes_written = send(fd, buffer+written, length - written, MSG_NOSIGNAL);

            if (bytes_written <= 0) {
                perror("write_exact()");

                return -1;
            }

            written += bytes_written;
        }

        return written;
    }

    static int write_bytes(int fd, char *buffer, size_t length) {
        int result = send(fd, buffer, length, MSG_NOSIGNAL);

        if (result <= 0) {
            if (errno != EPIPE) {
                perror("write_bytes()");
            }
        }

        return result;
    }

    static bool is_disconnected(int fd) {
        /*
         * \name is_disconnected()
         * \brief Checks if client has disconnected by calling recv with the MSG_PEEK flag
         */

        uint8_t u8;

        int bytes_read = recv(fd, &u8, sizeof(u8), MSG_PEEK);

        if (bytes_read == -1 && errno == EINPROGRESS) {
            return false;
        }

        return bytes_read <= 0;
    }

    static void set_reuse(int fd) {
        int y = 1;

        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(y));
    }
}

static void dynamic_assert(bool expr, const char *message) {
    if (!expr) {
        throw std::runtime_error(message);
    }
}

#endif //CAMELEO_NET_HELPERS_H
