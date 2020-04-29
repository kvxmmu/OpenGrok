//
// Created by kvxmmu on 4/27/20.
//

#include <iostream>
#include "selector.hpp"

Selector::Selector(int _max_listen) : epfd(epoll_create(_max_listen)), max_listen(_max_listen) {

}

void Selector::add(int fd, int events) const {
    Selector::set_nonblock(fd);
    epoll_event ev{};
    ev.events = events;
    ev.data.fd = fd;
    epoll_ctl(this->epfd, EPOLL_CTL_ADD, fd, &ev);
}

void Selector::wait(size_t &nfds, epoll_event *events, int timeout) const {
    nfds = epoll_wait(this->epfd, events, this->max_listen, timeout);
}

void Selector::remove(int fd) {
    close(fd);
}

void Selector::set_nonblock(int fd) {
    const int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void Selector::change_events(int fd, int new_events) const {
    epoll_event ev{};
    ev.data.fd = fd;
    ev.events = new_events;
    epoll_ctl(this->epfd, EPOLL_CTL_MOD, fd, &ev);
}
