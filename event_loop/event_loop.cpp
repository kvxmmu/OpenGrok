//
// Created by kvxmmu on 3/25/21.
//

#include "event_loop.hpp"


//// SendQueue

bool SendQueue::perform(int fd) {
    if (!this->is_queue_exists(fd)) {
        return true;
    }

    auto &queue = this->get_queue(fd);

    if (queue.empty()) {
        this->queues.erase(fd);

        printf("[GrokLoop:SendQueue] Queue is empty\n");

        return true;
    }

    auto &front = queue.front();

    int written = write_bytes(fd, front.buffer+front.received,
            front.length - front.received);

    if (written < 0) {
        front.received = front.length;

        perror("SendQueue::perform()");
    } else {
        front.received += written;
    }

    bool done = front.received >= front.length;
    bool is_empty = false;

    if (done) {
        this->allocator->deallocate(front.buffer, front.length);

        queue.pop_front();
        is_empty = this->remove_if_empty(queue, fd);
    }

    return is_empty;
}

void SendQueue::clear_queue(int fd) {
    if (!this->is_queue_exists(fd)) {
        return;
    }

    auto &queue = this->get_queue(fd);

    for (auto &item : queue) {
        this->allocator->deallocate(item.buffer, item.length);
    }

    queue.clear();
    this->queues.erase(fd);
}

void SendQueue::push(int fd, char *buffer, size_t length) {
    SendItem item(this->allocator->allocate(length), length);
    memcpy(item.buffer, buffer, length);

    if (!this->is_queue_exists(fd)) {
        this->queues[fd] = sendqueue_t();
    }

    auto &queue = this->get_queue(fd);

    queue.push_back(item);
}

//// GrokLoop

[[maybe_unused]] void GrokLoop::add_observer(AbstractObserver *observer) {
    observer->init(this);

    this->selector.add(observer->sockfd, observer->is_server ? EPOLLIN : EPOLLIN | EPOLLOUT);
    this->observers[observer->sockfd] = observer;

    observer->post_init();
}

void GrokLoop::map_client(int fd, AbstractObserver *observer) {
    this->mapped_clients[fd] = observer;
}

void GrokLoop::unmap_client(int fd) {
    this->mapped_clients.erase(fd);
}

void GrokLoop::run() {
    this->running = true;

    while (this->running) {
        if (this->observers.empty()) {
            throw Exceptions::NoObservers();
        }

        auto nfds = this->selector.wait();

        for (size_t nfd = 0; nfd < nfds; nfd++) {
            auto &event = this->selector.events_buffer[nfd];
            auto fd = event.data.fd;

            if (this->observers.find(fd) != this->observers.end()) {
                auto observer = this->observers.at(fd);

                if (!observer->is_server) {

                    if (event.events & EPOLLOUT) {
                        if (!observer->is_connected) {
                            observer->on_connect();

                            if (this->queue.is_queue_exists(fd) && this->queue.get_queue(fd).empty()) {
                                this->selector.remove(fd, EPOLLOUT);
                            }

                            continue;
                        }

                        bool remove_event = this->queue.perform(fd);

                        if (remove_event) {
                            this->selector.remove(fd, EPOLLOUT);
                        }

                        continue;
                    } else if (event.events & EPOLLIN) {
                        if (is_disconnected(fd)) {
                            observer->on_disconnect(fd);

                            this->selector.remove(fd, 0, true);
                            close(fd);
                            this->observers.erase(fd);
                            this->queue.clear_queue(fd);

                            continue;
                        }

                        this->handle_futures(fd);
                    } else {
                        throw Exceptions::UnknownEpollEvent(event.events);
                    }

                }

                auto new_fd = observer->on_connect();

                this->selector.add(new_fd, EPOLLIN);
                this->map_client(new_fd, observer);
            } else if (this->mapped_clients.find(fd) != this->mapped_clients.end()) {
                auto observer = this->mapped_clients.at(fd);

                if (event.events & EPOLLOUT) {
                    bool remove_event = this->queue.perform(fd);

                    if (remove_event) {
                        this->selector.remove(fd, EPOLLOUT);
                    }

                    continue;
                }

                if (is_disconnected(fd)) {
                    observer->on_disconnect(fd);

                    this->selector.remove(fd, 0, true);
                    close(fd);
                    this->unmap_client(fd);
                    this->queue.clear_queue(fd);

                    continue;
                }

                this->handle_futures(fd);
            }
        }
    }
}

void GrokLoop::remove_futures_if_empty(int fd) {
    if (this->futures.at(fd).empty()) {
        this->futures.erase(fd);
    }
}

void GrokLoop::handle_futures(int fd) {
    if (this->futures.find(fd) == this->futures.end()) {
        throw Exceptions::NoFutures(fd);
    }

    auto &queue = this->futures.at(fd);

    if (queue.empty()) {
        this->futures.erase(fd);

        throw Exceptions::NoFutures(fd);
    }

    auto &front = queue.front();

    if (front.type == Future::READ) {
        int bytes_read = read_bytes(fd, front.buffer+front.received,
                front.capacity - front.received);

        if (bytes_read < 0) {
            throw Exceptions::NotEnoughBytes(1, -1); // at least one!
        }

        front.received += bytes_read;

        if (front.received >= front.capacity) {
            front.callback(&front);
            front.dealloc();

            queue.pop_front();
        }
    } else if (front.type == Future::CAPTURE) {
        front.callback(&front);

        if (!front.pending) {
            queue.pop_front();
        }
    }
}

void GrokLoop::recv(int fd, size_t count, const future_callback_t &callback) {
    Future future(Future::READ, fd, &this->allocator, callback);
    future.alloc(count);

    if (this->futures.find(fd) == this->futures.end()) {
        this->futures[fd] = {};
    }

    auto &futures_q = this->futures[fd];
    futures_q.push_back(future);
}

void GrokLoop::capture(int fd, const future_callback_t &callback) {
    Future future(Future::CAPTURE, fd, &this->allocator, callback);

    if (this->futures.find(fd) == this->futures.end()) {
        this->futures[fd] = {};
    }

    auto &futures_q = this->futures[fd];
    futures_q.push_back(future);
}

void GrokLoop::send(int fd, char *buffer, size_t length) {
    this->queue.push(fd, buffer, length);

    this->selector.add(fd, EPOLLOUT);
}

void GrokLoop::force_disconnect(int fd) {
    this->selector.remove(fd, 0, true);
    close(fd);
    this->unmap_client(fd);
    this->queue.clear_queue(fd);
}

void GrokLoop::remove_observer(int fd) {
    auto observer = this->observers.at(fd);
    std::vector<int> fds;
    fds.reserve(this->mapped_clients.size() >> 2u);

    for (auto mapped_client : this->mapped_clients) {
        if (mapped_client.second == observer) {
            fds.push_back(mapped_client.first);
        }
    }

    for (auto cfd : fds) {
        this->force_disconnect(fd);
    }

    this->selector.remove(observer->sockfd, 0, true);
    close(observer->sockfd);
    this->queue.clear_queue(observer->sockfd);
}

