//
// Created by nero on 12.06.2021.
//

#include "loop.hpp"

Queues &Loop::gather_queue(sock_t fd) {
    if (queues.find(fd) == queues.end()) {
        queues[fd] = Queues();
    }

    return queues[fd];
}

void Loop::link_client(sock_t sock, IObserver *observer) {
    observer->clients.insert(sock);
    linked_clients[sock] = observer;
}

void Loop::unlink_client(sock_t sock, IObserver *observer,
                         bool erase) {
    if (erase) observer->clients.erase(sock);
    linked_clients.erase(sock);
}

void Loop::force_disconnect(sock_t client, bool erase,
                            bool call_on_disconnect) {
    if (linked_clients.find(client) != linked_clients.end()) {
        auto &observer = linked_clients[client];

        if (call_on_disconnect) observer->on_disconnect(client);

        this->remove_queues(client);
        this->unlink_client(client, observer,
                            erase);
        selector.remove(client);
        tcp_close(client);

        return;
    } else if (observers.find(client) == observers.end()) {
        return;
    }

    auto observer = observers.at(client);
    observer->on_disconnect(client);

    this->remove_observer(observer);
}

void Loop::perform_read_queue(sock_t sock, IObserver *observer) {
    auto &queue = this->gather_queue(sock);
    auto &read_queue = queue.read;

    if (read_queue.empty()) {
        EmptyQueue::raise(EmptyQueue::READ);
    }

    auto &front = read_queue.front();

    if (front.type == ReadItem::CAPTURE) {
        // capture callback
        observer->on_received(front.state, front);

        if (!front.pending) {
            read_queue.pop_front();
        }

        return;
    }

    auto read_ = tcp_recv(sock, front.buffer+front.received,
                          front.length - front.received);

    if (read_ <= 0) {
        // we will wait for epoll hup
        queue.clear_all();

        return;
    }

    front.received += read_;

    if (front.received >= front.length) {
        observer->on_received(front.state, front);

        delete[] front.buffer;
        read_queue.pop_front();
    }
}

void Loop::perform_write_queue(sock_t sock) {
    auto &queue = this->gather_queue(sock);
    auto &write_queue = queue.write;

    if (write_queue.empty()) {
        EmptyQueue::raise(EmptyQueue::WRITE);
    }

    auto &front = write_queue.front();
    auto sent = tcp_send(sock, front.buffer+front.sent,
                         front.length - front.sent);

    if (sent <= 0) {
        // We will wait for epoll hup
        selector.modify(sock, EVENTS_R);
        queue.clear_all();

        return;
    }

    front.sent += sent;

    if (front.sent >= front.length) {
        delete[] front.buffer;

        write_queue.pop_front();
    }

    if (write_queue.empty()) {
        selector.modify(sock, EVENTS_R);
    }
}

void Loop::add_observer(IObserver *observer) {
    auto fd = observer->socket;
    observer->init(this);

    if (!observer->is_server) {
        selector.add(fd, EVENTS_R | EVENT_WRITE);
    } else {
        selector.add(fd, EVENTS_R);
    }

    observer->post_init();
    observers[fd] = observer;
}

void Loop::remove_observer(IObserver *observer) {
    if (observer->is_server) {
        for (auto &client : observer->clients) {
            this->force_disconnect(client, false);
        }

        observer->clients.clear();
    } else {
        this->remove_queues(observer->socket);
    }

    observers.erase(observer->socket);
    tcp_close(observer->socket);
}

void Loop::capture(sock_t sock, state_t state) {
    auto &read_queue = this->gather_queue(sock).read;
    read_queue.emplace_back(sock, state);
}

void Loop::recv(sock_t sock, size_t length, state_t state,
                uint64_t u64) {
    auto &read_queue = this->gather_queue(sock).read;
    read_queue.emplace_back(length, sock, state,
                            u64);
}

void Loop::send(sock_t sock, char *buffer, size_t length) {
    auto &write_queue = this->gather_queue(sock).write;

    if (write_queue.empty()) {
        selector.modify(sock, EVENTS_R | EVENT_WRITE);
    }

    write_queue.emplace_back(buffer, length);
}

void Loop::remove_queues(sock_t sock) {
    auto &q = this->gather_queue(sock);
    q.clear_all();

    queues.erase(sock);
}

void Loop::stop() {
    this->running = false;
}

void Loop::run() {
    running = true;

    while (running) {
        if (observers.empty()) {
            throw NoObservers("No observers in loop");
        }

        auto n_events = selector.wait();

        if (n_events == -1) {
            perror("selector.wait()");

            continue;
        }

        for (size_t pos = 0; pos < n_events; pos++) {
            auto &ev = selector.events[pos];
            auto fd = static_cast<sock_t>(ev.data.u64);
            auto events = ev.events;

            if ((events & EPOLLHUP) || (events & EPOLLRDHUP)) {
                decltype(observers)::iterator obs_iter;

                if ((obs_iter = observers.find(fd)) != observers.end()) {
                    auto &observer = obs_iter->second;
                    observer->on_disconnect(fd);

                    if (!observer->is_server) {
                        this->remove_observer(observer);
                    }

                    break;
                }

                this->force_disconnect(fd, true, true);

                break;
            }

            if (events & EPOLLIN) {
                auto obs_iterator = observers.find(fd);

                if (obs_iterator != observers.end()) {
                    auto &observer = obs_iterator->second;

                    if (observer->is_server) {
                        auto client_fd = observer->on_connect();
                        selector.add(client_fd, EVENTS_R);
                        this->link_client(client_fd, observer);

                        observer->post_connect(client_fd);
                    } else {
                        this->perform_read_queue(fd, observer);
                    }
                } else {
                    auto &linked_observer = linked_clients.at(fd);

                    this->perform_read_queue(fd, linked_observer);
                }
            }

            if (events & EPOLLOUT) {
                decltype(observers)::iterator obs_iter;

                if ((obs_iter = observers.find(fd)) != observers.end()) {
                    // client connection
                    auto &observer = obs_iter->second;

                    if (!observer->is_connected) {
                        observer->on_connect();
                        observer->is_connected = true;

                        selector.modify(fd, EVENTS_R);
                        observer->post_connect(fd);

                        continue;
                    }
                }

                this->perform_write_queue(fd);
            }
        }
    }
}

void Loop::handle_selector_error(sock_t fd) {
    if (errno != EBADF) {
        perror("selector error");
    }
}
