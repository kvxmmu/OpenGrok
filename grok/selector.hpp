//
// Created by kvxmmu on 4/27/20.
//

#ifndef OPENGROK_SELECTOR_HPP
#define OPENGROK_SELECTOR_HPP

#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>

#include <vector>

#define DEFAULT_MAX_LISTEN 4096

class Selector {
public:
    int epfd;
    int max_listen;

    explicit Selector(int _max_listen = DEFAULT_MAX_LISTEN);

    static void set_nonblock(int fd);
    void add(int fd, int events = EPOLLIN) const;

    void wait(size_t &nfds, epoll_event *events, int timeout) const;

    void change_events(int fd, int new_events) const;

    static void remove(int fd);
};


#endif //OPENGROK_SELECTOR_HPP
