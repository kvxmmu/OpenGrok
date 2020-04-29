//
// Created by kvxmmu on 4/28/20.
//

#include <iostream>
#include "evloop.hpp"


EventLoop::EventLoop() : selector(EOPMAX_EVENTS) {

}

Future &EventLoop::add_callback(int fd, void (*tick)(EventLoop *, Future *)) {
    this->selector.add(fd, EPOLLIN);
    Future new_fut(fd, tick);
    this->callbacks.push_back(new_fut);
    return this->callbacks.at(this->callbacks.size()-1);
}

void EventLoop::poll() {
    this->running = true;

    std::vector<Future*> temp_futures;

    epoll_event events[EOPMAX_EVENTS];
    size_t nfds;

    while (this->running) {
        temp_futures.clear();
        this->selector.wait(nfds, events, TIMEOUT);
        for (size_t pos = 0; pos < nfds; pos++) {
            epoll_event &ev = events[pos];

            if (this->in_futures(ev, temp_futures)) {
                for (Future *temp_fut : temp_futures) {
                    if (temp_fut->tick != nullptr && (temp_fut->trigger_event == -1 || ev.events & temp_fut->trigger_event)) {
                        temp_fut->tick(this, temp_fut);
                    }
                    if (temp_fut->finished && !temp_fut->removed)
                        this->remove_by_fd(temp_fut->fd, temp_fut->trigger_event);
                }
            } else {
                std::cerr << "[OpenGrok] EventLoop Error: unknown FD#" << ev.data.fd << std::endl;
                epoll_ctl(this->selector.epfd, EPOLL_CTL_DEL, ev.data.fd, &ev);
            }
        }
    }
}

void EventLoop::change_trigger(int fd, int trigger) const {
    this->selector.change_events(fd, trigger);
}

bool EventLoop::in_futures(epoll_event &ev, std::vector<Future*> &futs) {
    bool found = false;

    for (auto &ifut : this->callbacks) {
        if (ifut.fd == ev.data.fd) {
            futs.push_back(&ifut);
            found = true;
        }
    }
    return found;
}


Future::Future(int sfd, void (*on_tick)(EventLoop *, Future *)) : fd(sfd), tick(on_tick) {

}

void Future::finish(EventLoop *evloop) {
    // std::cout << "finish(" << evloop << ")" << std::endl;
    this->finished = true;
    if (evloop != nullptr) {
        evloop->remove_by_fd(this->fd, this->trigger_event);
        this->removed = true;
    }
    if (this->on_finished != nullptr)
        this->on_finished(evloop, this);
    epoll_event ev{};
    ev.data.fd = this->fd;
    ev.events = this->trigger_event;
    epoll_ctl(evloop->selector.epfd, EPOLL_CTL_DEL, this->fd, &ev);
}

void Future::modify_events(int new_events, EventLoop *loop) {
    this->trigger_event = new_events;
    loop->change_trigger(this->fd, new_events);
}