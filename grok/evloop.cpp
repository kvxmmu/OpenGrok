//
// Created by kvxmmu on 4/28/20.
//

#include <iostream>
#include "evloop.hpp"


EventLoop::EventLoop() : selector(EOPMAX_EVENTS) {

}

Future &EventLoop::add_callback(int fd, void (*tick)(EventLoop *, Future *)) {
    this->selector.add(fd);
    Future new_fut(fd, tick);
    this->callbacks[fd] = new_fut;
    return this->callbacks.at(fd);
}

void EventLoop::poll() {
    this->running = true;

    Future temp_fut(0, nullptr);

    epoll_event events[EOPMAX_EVENTS];
    size_t nfds;

    while (this->running) {
        this->selector.wait(nfds, events, TIMEOUT);
        for (size_t pos = 0; pos < nfds; pos++) {
            epoll_event &ev = events[pos];

            if (this->in_futures(ev, temp_fut)) {
                if (temp_fut.tick != nullptr && (temp_fut.trigger_event == 1 || ev.events & temp_fut.trigger_event))
                    temp_fut.tick(this, &temp_fut);
                if (temp_fut.finished && this->callbacks.find(ev.data.fd) != this->callbacks.end())
                    this->callbacks.erase(ev.data.fd);
            } else {
                std::cerr << "[OpenGrok] EventLoop Error: unknown FD#" << ev.data.fd << std::endl;
            }
        }
    }
}

void EventLoop::change_trigger(int fd, int trigger) const {
    this->selector.change_events(fd, trigger);
}

bool EventLoop::in_futures(epoll_event &ev, Future &fut) {
    for (auto &ifut : this->callbacks) {
        if (ifut.first == ev.data.fd) {
            fut = ifut.second;
            return true;
        }
    }
    return false;
}

Future::Future(int sfd, void (*on_tick)(EventLoop *, Future *)) : fd(sfd), tick(on_tick) {

}

void Future::finish(EventLoop *evloop) {
    this->finished = true;
    if (evloop != nullptr)
        evloop->callbacks.erase(this->fd);
    if (this->on_finished != nullptr)
        this->on_finished(evloop, this);
}

void Future::modify_events(int new_events, EventLoop *loop) const {
    loop->change_trigger(this->fd, new_events);
}
