//
// Created by kvxmmu on 4/28/20.
//

#ifndef OPENGROK_EVLOOP_HPP
#define OPENGROK_EVLOOP_HPP

#include <grok/selector.hpp>
#include <unordered_map>
#include <vector>


#define EOPMAX_EVENTS 4096
#define TIMEOUT 5000


class EventLoop;


class Future {
public:
    bool finished = false;

    void (*on_finished)(EventLoop *, Future *) = nullptr;
    void (*tick)(EventLoop *, Future *) = nullptr;
    void *memptr = nullptr;
    int state = 0;

    int fd;

    int trigger_event = EPOLLIN;

    void finish(EventLoop *evloop);

    Future() = default;
    Future(int sfd, void (*on_tick)(EventLoop *, Future *));

    void modify_events(int new_events, EventLoop *loop) const;

};

class EventLoop {
public:
    Selector selector;

    bool running = false;

    std::unordered_map<int, Future> callbacks;

    EventLoop();

    void change_trigger(int fd, int trigger) const;
    Future &add_callback(int fd, void (*tick)(EventLoop *, Future *) = nullptr);
    bool in_futures(epoll_event &ev, std::vector<Future> &futs);

    inline void remove_by_fd(int fd) {
        this->callbacks.erase(fd);
    }

    void poll();


};

#endif //OPENGROK_EVLOOP_HPP
