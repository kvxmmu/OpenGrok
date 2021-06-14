//
// Created by nero on 12.06.2021.
//

#ifndef GROKPP_EPOLL_H
#define GROKPP_EPOLL_H

#include "platform.h"


#ifdef _PLATFORM_WINDOWS
#include "wepoll.h"

#else
#include <sys/epoll.h>
#include <unistd.h>

typedef int epoll_t;

inline void epoll_close(epoll_t epfd) {
    close(epfd);
}

#endif

#endif //GROKPP_EPOLL_H
