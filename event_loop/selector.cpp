//
// Created by kvxmmu on 8/24/20.
//

#include "selector.hpp"
#include <cstdio>

/// Helpers

void set_nonblock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void set_reuse(int fd) {
    int y = 1;

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int));
}

/// EpollSelector


void EpollSelector::add(int fd, uint32_t events) {
    if (this->fds.find(fd) == this->fds.end()) {
        epoll_event ev{};
        ev.data.fd = fd;
        ev.events = events;

        set_nonblock(fd);

        if (epoll_ctl(this->epfd, EPOLL_CTL_ADD, fd, &ev) != 0) {
            perror("EpollSelector::add()");
        } else {
            this->fds[fd] = events;
        }

        return;
    }

    uint32_t old_events = this->fds.at(fd);

    this->modify(fd, old_events | events);
}

void EpollSelector::modify(int fd, uint32_t events) {
    this->fds[fd] = events;

    epoll_event ev{};
    ev.data.fd = fd;
    ev.events = events;

    if (epoll_ctl(this->epfd, EPOLL_CTL_MOD, fd, &ev) != 0) {
        perror("EpollSelector::modify");
    }
}

void EpollSelector::remove(int fd, uint32_t events,
                           bool just_remove) {
    uint32_t flags = this->fds.at(fd);

    if (just_remove || flags == events) {
        epoll_event ev{};
        ev.data.fd = fd;
        ev.events = this->fds.at(fd);

        if (epoll_ctl(this->epfd, EPOLL_CTL_DEL, fd, &ev) != 0) {
            perror("EpollSelector::remove");
        }

        this->fds.erase(fd);

        return;
    }

    this->modify(fd, this->fds.at(fd) & ~events);
}

size_t EpollSelector::wait(int timeout) {
    size_t nfds = epoll_wait(this->epfd, this->events_buffer, MAX_EPOLL_SIZE,
            timeout);

    return nfds;
}

bool EpollSelector::has_flag(int fd, uint32_t flag) {
    if (this->fds.find(fd) == this->fds.end()) {
        return false;
    }

    return (this->fds.at(fd) & flag) == flag;
}
