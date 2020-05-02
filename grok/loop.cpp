//
// Created by kvxmmu on 5/1/20.
//

#include <iostream>
#include "loop.hpp"

void set_nonblock(int fd) {
    const int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

EpollSelector::EpollSelector() {
    this->epfd = epoll_create(MAX_EPOLL_SIZE);
}

void EpollSelector::modify(int fd, uint32_t new_events) {
    epoll_event ev{};
    ev.data.fd = fd;
    ev.events = new_events;
    if (epoll_ctl(this->epfd, EPOLL_CTL_MOD, fd, &ev) != 0) {
        perror("epoll_ctl()");
    }
    this->fds[fd] = new_events;
}

void EpollSelector::wait(size_t &nfds, epoll_event *events) const {
    nfds = epoll_wait(this->epfd, events, MAX_EPOLL_SIZE, TIMEOUT);
}

void EpollSelector::add(int fd, uint32_t events) {
    if (this->fds.find(fd) != this->fds.end()) {
        this->modify(fd, events | this->fds.at(fd));
        return;
    }

    set_nonblock(fd);

    epoll_event ev{};
    ev.data.fd = fd;
    ev.events = events;
    if (epoll_ctl(this->epfd, EPOLL_CTL_ADD, fd, &ev) != 0) {
        perror("epoll_ctl()");
    }
    this->fds.emplace(std::make_pair(fd, events));
}

void EpollSelector::remove(int fd, uint32_t events) {
    epoll_event ev{};
    ev.events = events;
    ev.data.fd = fd;

    if (this->fds.find(fd) != this->fds.end()) {
        uint32_t eflags = this->fds[fd];
        if (events == JUST_REMOVE) {
            ev.events = this->fds[fd];
            epoll_ctl(this->epfd, EPOLL_CTL_DEL, fd, &ev);
            this->fds.erase(fd);
            return;
        } else if (eflags == events) {
            epoll_ctl(this->epfd, EPOLL_CTL_DEL, fd, &ev);
            this->fds.erase(fd);
            return;
        }
        this->modify(fd, eflags & ~events);
    }
}

Future::Future(int _fd, void (*_on_tick)(EventLoop *fut, Future *)) : fd(_fd), on_tick(_on_tick) {

}

void Future::update_activity() {
    this->last_active = time(nullptr);
}

time_t Future::get_time_delta() const {
    return time(nullptr) - this->last_active;
}

Future &EventLoop::create_future(int fd, void (*_callback)(EventLoop *fut, Future *), uint32_t events) {
    this->selector.add(fd);

    auto fut = new Future(fd, _callback);
    fut->events = events;
    fut->loop = this;

    this->futures.push_back(fut);
    return *this->futures[this->futures.size() - 1];
}

Future &EventLoop::insert_to_top(int fd, void (*_callback)(EventLoop *fut, Future *), uint32_t events) {
    this->selector.add(fd);

    auto fut = new Future(fd, _callback);
    fut->events = events;
    fut->loop = this;

    this->futures.insert(this->futures.begin(), fut);
    return *this->futures[0];
}

void EventLoop::run() {
    epoll_event events[MAX_EPOLL_SIZE];
    size_t nfds;
    this->running = true;

    std::vector<Future*> to_call;

    size_t epos;

    while (this->running) {
        this->selector.wait(nfds, events);

        for (size_t pos = 0; pos < nfds; pos++) {
            if (!to_call.empty())
                to_call.clear();
            epoll_event &ev = events[pos];

            if (this->in_queue(ev.data.fd, ev.events, epos)) {
                if (epos == std::string::npos)
                    continue;

                QueueItem &item = *this->packets_queue[epos];

                item.sent += write_bytes(ev.data.fd, item.buffer+item.sent, item.length-item.sent);

                if (item.on_progress != nullptr)
                    item.on_progress(&item);

                if (item.sent >= item.length) {
                    this->selector.remove(ev.data.fd, EPOLLOUT);
                    delete this->packets_queue[epos];
                    this->packets_queue.erase(this->packets_queue.begin()+epos);
                    this->enable_if_can(ev.data.fd);
                }
            } else if (this->in_futures(ev, to_call)) {
                for (Future *future_ptr : to_call) {
                    future_ptr->on_tick(this, future_ptr);
                }
                to_call.clear();
            } else {
                std::cout << NAME << SPACE << "Unknown FD#" << ev.data.fd << std::endl;
            }

        }
    }
}

bool EventLoop::in_futures(epoll_event &ev, std::vector<Future *> &futs) {
    bool found = false;

    for (Future *fut : this->futures) {
        if (fut->fd == ev.data.fd) {
            futs.push_back(fut);
            if (fut->skip_next)
                return true;
            found = true;
        }
    }

    return found;
}

QueueItem &EventLoop::push_packet(void *buffer, int length, int fd, size_t pos, bool skip_fd) {
    auto item = new QueueItem;
    item->length = length;
    item->skip_fd = skip_fd;
    item->fd = fd;
    memcpy(item->buffer, buffer, length);

    this->selector.add(fd, EPOLLOUT | EPOLLERR);

    auto iterator = this->packets_queue.begin();

    if (pos == std::string::npos) {
        this->packets_queue.push_back(item);
        return *this->packets_queue.back();
    } else if (pos == 0) {
        this->packets_queue.push_front(item);
        return *this->packets_queue.front();
    }

    this->packets_queue.insert(iterator+pos, item);
    return *this->packets_queue.at(pos);
}

bool EventLoop::in_queue(int fd, uint32_t events, size_t &epos) {
    for (size_t pos = 0; pos < this->packets_queue.size(); pos++) {
        QueueItem &item = *this->packets_queue.at(pos);
        if (item.fd == fd) {
            if (events & EPOLLERR) {
                close(item.fd);
                this->packets_queue.erase(this->packets_queue.begin()+pos);
                return false;
            }

            if (item.skip_fd && !(events & EPOLLOUT)) {
                epos = std::string::npos;
                return true;
            } else if (!(events & EPOLLOUT))
                return false;
            epos = pos;
            return true;
        }
    }
    return false;
}

void EventLoop::enable_if_can(int fd) {
    for (QueueItem *item : this->packets_queue) {
        if (item->fd == fd) {
            this->selector.add(item->fd, EPOLLOUT);
            return;
        }
    }
}

void EventLoop::remove_by_fd(int fd, uint32_t events) {
    size_t index = std::string::npos;

    for (size_t pos = 0; pos < this->futures.size(); pos++) {
        Future &fut = *this->futures[pos];

        if (fut.fd == fd && (fut.events == events || events == JUST_REMOVE)) {
            this->selector.remove(fd, JUST_REMOVE);

            if (fut.on_finish != nullptr)
                fut.on_finish(this, &fut);

            index = pos;
            break;
        }
    }

    if (index != std::string::npos) {
        delete this->futures[index];
        this->futures.erase(this->futures.begin()+index);
    } else {
        std::cout << NAME << SPACE << "Unknown RemoveByFd FD#" << fd << std::endl;
    }
}

void EventLoop::finish(Future *fut) {
    if (fut->on_finish != nullptr)
        fut->on_finish(fut->loop, fut);
    this->selector.remove(fut->fd, fut->events);
    this->remove_by_fd(fut->fd, fut->events);
}

