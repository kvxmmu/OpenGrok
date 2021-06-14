//
// Created by nero on 12.06.2021.
//

#include "selector.hpp"

void Selector::ctl(sock_t sock, int op, uint32_t evs) const {
    epoll_event ev_struct{};
    ev_struct.events = evs;
    ev_struct.data.u64 = sock;

    auto result = epoll_ctl(epfd, op, sock, &ev_struct);

    if (result != 0) {
#ifdef THROW_ERRORS
        throw SelectorError();
#else
        perror("epoll_ctl()");
#endif
    }
}

void Selector::add(sock_t sock, uint32_t evs) const {
    this->ctl(sock, EPOLL_CTL_ADD, evs);
}

void Selector::modify(sock_t sock, uint32_t evs) const {
    this->ctl(sock, EPOLL_CTL_MOD, evs);
}

void Selector::remove(sock_t sock) const {
    this->ctl(sock, EPOLL_CTL_DEL, 0xffffffffu);
}

int Selector::wait(int timeout) {
    return epoll_wait(epfd, events, MAX_EVENTS,
                      timeout);
}
