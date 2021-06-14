//
// Created by nero on 12.06.2021.
//

#ifndef GROKPP_SELECTOR_HPP
#define GROKPP_SELECTOR_HPP

#include <stdexcept>
#include <cstring>
#include <cstdio>

#include "epoll.h"
#define MAX_EVENTS 4096

#define THROW_ERRORS

class SelectorError : public std::runtime_error {
public:
    SelectorError() : std::runtime_error(strerror(errno)) {

    }
};


class Selector {
public:
    epoll_t epfd;
    epoll_event events[MAX_EVENTS]{};

    Selector() : epfd(epoll_create(MAX_EVENTS)) {

    }

    void ctl(sock_t sock, int op,
             uint32_t evs) const;

    void add(sock_t sock, uint32_t evs) const;
    void modify(sock_t sock, uint32_t evs) const;
    void remove(sock_t sock) const;
    int wait(int timeout = -1);
};


#endif //GROKPP_SELECTOR_HPP