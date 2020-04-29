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
    bool removed = false;

    void (*on_finished)(EventLoop *, Future *) = nullptr;
    void (*tick)(EventLoop *, Future *) = nullptr;
    void *memptr = nullptr;
    int state = 0;

    int fd;

    int trigger_event = EPOLLIN;

    void finish(EventLoop *evloop);

    Future() = default;
    Future(int sfd, void (*on_tick)(EventLoop *, Future *));

    void modify_events(int new_events, EventLoop *loop);
};

class EventLoop {
public:
    Selector selector;

    bool running = false;

    std::vector<Future> callbacks;

    EventLoop();

    void change_trigger(int fd, int trigger) const;
    Future &add_callback(int fd, void (*tick)(EventLoop *, Future *) = nullptr);
    bool in_futures(epoll_event &ev, std::vector<Future*> &futs);

    inline void remove_by_fd(int fd, int events = -1) {
        size_t index = std::string::npos;
        for (size_t pos = 0; pos < this->callbacks.size(); pos++) {
            Future &fut = this->callbacks[pos];
            if (fut.fd == fd && (events == -1 || fut.trigger_event == events)) {
                index = pos;
                break;
            }
        }

        if (index != std::string::npos)
            this->callbacks.erase(this->callbacks.begin()+index);
    }

    void poll();


};

#endif //OPENGROK_EVLOOP_HPP
