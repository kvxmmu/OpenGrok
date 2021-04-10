//
// Created by kvxmmu on 3/27/21.
//

#ifndef CAMELEO_SELECTOR_HPP
#define CAMELEO_SELECTOR_HPP

#include <sys/epoll.h>
#include <unistd.h>

#ifndef CAMELEO_DONT_HANDLE_EPOLL_ERRORS
#   include <cstdio>
#endif

#include <stdexcept>
#include <fcntl.h>

namespace Cameleo {
    constexpr static size_t max_epoll_events = 64;

    class EpollSelector {
    public:
        int epfd;

        /*
         * EpollSelector without storing descriptor's events
         */

        epoll_event events[max_epoll_events];

        EpollSelector() : epfd(epoll_create(max_epoll_events)) {

        }

        void add(int fd, uint32_t levents,
                 void *ptr = nullptr, uint32_t u32 = 0,
                 uint64_t u64 = 0) const;
        void remove(int fd) const;
        void modify(int fd, uint32_t levents,
                    void *ptr = nullptr, uint32_t u32 = 0,
                    uint64_t u64 = 0) const;

        [[nodiscard]] int wait(int timeout = -1);

        ~EpollSelector();
    };
}


#endif //CAMELEO_SELECTOR_HPP
