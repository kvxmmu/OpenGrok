//
// Created by kvxmmu on 3/27/21.
//

#include "selector.hpp"

void Cameleo::EpollSelector::add(int fd, uint32_t levents,
                                 void *ptr, uint32_t u32,
                                 uint64_t u64) const {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);

    epoll_event event{};
    event.data.fd = fd;
    event.events = levents;

    auto ev = EPOLLIN;

#ifndef CAMELEO_DONT_HANDLE_EPOLL_ERRORS
    if (epoll_ctl(this->epfd, EPOLL_CTL_ADD,
            fd, &event) != 0) {
        perror("Cameleo::EpollSelector::add()");
    }
#else
    epoll_ctl(this->epfd, EPOLL_CTL_ADD, fd, &event);
#endif
}

void Cameleo::EpollSelector::remove(int fd) const {
    epoll_event ev{};
    ev.data.fd = fd;
    ev.events = 0xffffffff;

#ifndef CAMELEO_DONT_HANDLE_EPOLL_ERRORS
    if (epoll_ctl(this->epfd, EPOLL_CTL_DEL, fd, &ev) != 0) {
        perror("Cameleo::EpollSelector::remove()");
    }
#else
    epoll_ctl(this->epfd, EPOLL_CTL_DEL, fd, &ev);
#endif
}

void Cameleo::EpollSelector::modify(int fd, uint32_t levents,
                                    void *ptr, uint32_t u32,
                                    uint64_t u64) const {
    epoll_event ev{};
    ev.data.fd = fd;
    ev.events = levents;

#ifndef CAMELEO_DONT_HANDLE_EPOLL_ERRORS
    if (epoll_ctl(this->epfd, EPOLL_CTL_MOD, fd, &ev) != 0) {
        throw std::runtime_error("Cameleo::EpollSelector::modify()");
    }
#else
    epoll_ctl(this->epfd, EPOLL_CTL_MOD, fd, &ev);
#endif

}

Cameleo::EpollSelector::~EpollSelector() {
    if (close(this->epfd) != 0) {
        perror("Cameleo::EpollSelector::~EpollSelector()");
    }
}

int Cameleo::EpollSelector::wait(int timeout) {
    return epoll_wait(this->epfd, this->events, Cameleo::max_epoll_events,
                      timeout);
}
