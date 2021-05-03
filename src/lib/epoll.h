//
// Created by nero on 5/2/21.
//

#ifndef OPENGROK_EPOLL_H
#define OPENGROK_EPOLL_H

#include "platform.h"

#ifdef _PLATFORM_WINDOWS
# include "wepoll.h"

typedef HANDLE epoll_t ;
typedef SOCKET socket_t;

static void close_epoll(epoll_t handle) {
    epoll_close(handle);
}
#else
# include <sys/epoll.h>
# include <unistd.h>

typedef int epoll_t ;
typedef int socket_t;

static void close_epoll(epoll_t handle) {
    close(handle);
}
#endif

#include <stdio.h>
#include <errno.h>

static uint64_t hardcast_to_u64(socket_t handle) {
    uint64_t u64 = 0;

    memcpy(&u64, &handle, sizeof(socket_t));

    return u64;
}

static void epoll_ctl_impl(epoll_t epoll, socket_t handle,
               int op, uint32_t flags, const char *description) {
    struct epoll_event event;
    event.data.u64 = hardcast_to_u64(handle);
    event.events = flags;

    if (epoll_ctl(epoll, op, handle, &event) != 0) {
        fprintf(stderr, "%s: %s\n", description, strerror(errno));
#ifdef EPOLL_THROW
        abort();
#endif
    }
}

static void epoll_add(epoll_t epoll, socket_t handle,
               uint32_t flags) {
    epoll_ctl_impl(epoll, handle, EPOLL_CTL_ADD,
                   flags, "epoll_add()");
}

static void epoll_modify(epoll_t epoll, socket_t handle,
                  uint32_t flags) {
    epoll_ctl_impl(epoll, handle, EPOLL_CTL_MOD,
                   flags, "epoll_modify()");
}

static void epoll_remove(epoll_t epoll, socket_t handle) {
    epoll_ctl_impl(epoll, handle, EPOLL_CTL_DEL,
                   0xffffffff, "epoll_remove()");
}

#endif //OPENGROK_EPOLL_H
