//
// Created by kvxmmu on 3/27/21.
//

#include "loop.hpp"

#include <utility>

void Cameleo::EventLoop::add_observer(Cameleo::IObserver *observer) {
    /*
     * \name add_observer()
     * \brief initializes observer and registering his descriptor
     */

    observer->init(this);

    auto sockfd = observer->sockfd;
    this->selector.add(sockfd, observer->is_server ? EPOLLIN : EPOLLIN | EPOLLOUT);

    observer->post_init();

    this->observers[sockfd] = observer;
}

void Cameleo::EventLoop::run() {
    /*
     * \name run()
     * \brief setting variable running to true, then passing events to observers
     */

    this->running = true;

    while (this->running) {
        int nfds = this->selector.wait();

        if (nfds < 0) {
            throw std::runtime_error(strerror(errno));
        }

        for (size_t nfd = 0; nfd < nfds; nfd++) {
            epoll_event &event = this->selector.events[nfd];
            auto fd = event.data.fd;

            if (event.events & EPOLLOUT) {
                if (this->observers.find(fd) != this->observers.end()) {
                    auto observer = this->observers[fd];

                    if (!observer->is_connected) {
                        observer->on_connect();
                        this->selector.modify(fd, EPOLLIN);

                        observer->post_connect(fd);

                        continue;
                    }
                }

                auto need_remove = this->send_queue.perform(fd);

                if (need_remove) {
                    this->selector.modify(fd, EPOLLIN);
                }
            }

            if (event.events & EPOLLIN) {
                if (this->observers.find(fd) != this->observers.end()) {
                    auto observer = this->observers[fd];

                    if (observer->is_server) {
                        auto cfd = observer->on_connect();
                        observer->__add_client(cfd);

                        this->selector.add(cfd, EPOLLIN);

                        observer->post_connect(cfd);

                        continue;
                    } else if (Net::is_disconnected(fd)) {
                        observer->on_disconnect();
                        this->remove_observer(observer);

                        continue;
                    }

                    this->handle_futures(fd);

                    continue;
                }

                if (Net::is_disconnected(fd)) {
                    for (auto obs_pair : this->observers) {
                        auto observer = obs_pair.second;

                        if (observer->__has_client(fd)) {
                            observer->on_disconnect();
                            this->force_disconnect(fd);
                            observer->__clients.erase(fd);

                            break;
                        }
                    }

                    continue;
                }

                this->handle_futures(fd);

                continue;
            }

            if (!(event.events & EPOLLIN) && !(event.events & EPOLLOUT)) {
                throw std::logic_error("Unknown event type");
            }
        }
    }
}

void Cameleo::EventLoop::force_disconnect(int fd) {
    this->selector.remove(fd);
    close(fd);

    this->clear_futures(fd);
}

void Cameleo::EventLoop::remove_observer(Cameleo::IObserver *observer) {
    if (!observer->is_server) {
        this->force_disconnect(observer->sockfd);
        this->observers.erase(observer->sockfd);

        delete observer;

        return;
    }

    for (auto client : observer->__clients) {
        this->force_disconnect(client);
    }

    observer->__clients.clear();
    delete observer;
}

//// Futures

void Cameleo::EventLoop::remove_if_empty_fut(int fd) {
    if (!this->futures_exists(fd)) {
        return;
    }

    auto &queue = this->futures[fd];

    if (queue.empty()) {
        this->futures.erase(fd);
    }
}

void Cameleo::EventLoop::clear_futures(int fd) {
    if (!this->futures_exists(fd)) {
        return;
    }

    auto &queue = this->futures[fd];

    for (auto &future : queue) {
        if (future->recv_buffer != nullptr) {
            future->deallocate();
        }
    }

    queue.clear();
    this->futures.erase(fd);
}

void Cameleo::EventLoop::send(int fd, char *buffer, size_t length) {
    /*
     * \name send()
     * \brief Writes ALL data to given fd
     */

    this->send_queue.push(fd, buffer, length);
    this->selector.modify(fd, EPOLLIN | EPOLLOUT);
}

void Cameleo::EventLoop::recv(int fd, size_t count,
        const Future::callback_t &callback) {
    auto future = std::make_shared<Future>(fd, Future::READ,
            &this->allocator, callback);

    this->create_futures_if_empty(fd);
    auto &queue = this->futures[fd];

    future->allocate(count);
    queue.push_back(future);
}

void Cameleo::EventLoop::capture(int fd, const Cameleo::Future::callback_t &capture_func) {
    auto future = std::make_shared<Future>(fd, Future::CAPTURE,
                  &this->allocator, capture_func);

    this->create_futures_if_empty(fd);
    auto &queue = this->futures[fd];

    queue.push_back(future);
}

//// Helpers

bool Cameleo::EventLoop::futures_exists(int fd) {
    return this->futures.find(fd) != this->futures.end();
}

void Cameleo::EventLoop::create_futures_if_empty(int fd) {
    if (!this->futures_exists(fd)) {
        this->futures[fd] = futures_t();
    }
}

void Cameleo::EventLoop::handle_futures(int fd) {
    using namespace Cameleo::Net;

    if (this->futures.find(fd) == this->futures.end() || this->futures.at(fd).empty()) {
        throw std::logic_error("No futures in queue");
    }

    auto &queue = this->futures[fd];
    auto &front = *queue.front();

    switch (front.type) {
        case Future::READ: {
            int bytes_read = read_bytes(fd, front.recv_buffer+front.received,
                    front.capacity - front.received);

            if (bytes_read == -1) {
                front.deallocate();
                front.complete();
                queue.pop_front();

                break;
            }

            front.received += bytes_read;

            if (front.received >= front.capacity) {
                front.callback(&front);

                front.deallocate();
                front.complete();
                queue.pop_front();
            }

            break;
        }

        case Future::CAPTURE: {
            front.callback(&front);

            if (!front.pending) {
                front.complete();
                queue.pop_front();
            }

            break;
        }

    }

    this->remove_if_empty_fut(fd);
}
