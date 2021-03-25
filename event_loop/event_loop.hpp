//
// Created by kvxmmu on 3/25/21.
//

#ifndef GROKLOOP_EVENT_LOOP_HPP
#define GROKLOOP_EVENT_LOOP_HPP

#include "abstract.hpp"
#include "selector.hpp"

#include <unordered_map>
#include <memory>
#include <deque>

#include <functional>
#include <cstring>
#include <cstdio>

#include "inet.hpp"

class Future;
class SendItem;

using sendqueue_t = std::deque<SendItem>;
using future_callback_t = std::function<void(Future *)>;

class Future {
public:
    enum FutureType {
        READ, CAPTURE
    };

    FutureType type;

    std::allocator<char> *allocator;

    char *buffer = nullptr;
    size_t capacity = 0;

    future_callback_t callback;

    Future(FutureType _type,
            std::allocator<char> *_allocator,
            future_callback_t _callback) : type(_type), allocator(_allocator), callback(std::move(_callback)) {

    }

    void alloc(size_t cap) {
        this->buffer = this->allocator->allocate(cap);
        this->capacity = cap;
    }

    void dealloc() {
        this->allocator->deallocate(this->buffer, this->capacity);

        this->buffer = nullptr;
        this->capacity = 0;
    }
};

class SendItem {
public:
    size_t length;
    size_t received;

    char *buffer;

    explicit SendItem(char *_buffer, size_t _length) : buffer(_buffer), length(_length), received(0) {

    }
};

class SendQueue {
public:
    std::unordered_map<int, sendqueue_t> queues;
    std::allocator<char> *allocator;

    explicit SendQueue(std::allocator<char> *_allocator) : allocator(_allocator) {

    }

    inline bool is_queue_exists(int fd) {
        return this->queues.find(fd) != this->queues.end();
    }

    inline sendqueue_t &get_queue(int fd) {
        return this->queues.at(fd);
    }

    inline bool remove_if_empty(int fd) {
        if (this->get_queue(fd).empty()) {
            this->queues.erase(fd);

            return true;
        }

        return false;
    }

    inline void remove_if_empty(sendqueue_t &queue, int fd) {
        if (queue.empty()) {
            this->queues.erase(fd);
        }
    }

    void push(int fd, char *buffer, size_t length);
    bool perform(int fd);

    void clear_queue(int fd);
};

class GrokLoop {
public:
    EpollSelector selector;

    std::allocator<char> allocator;
    SendQueue queue;

    std::unordered_map<int, Future> futures;
    std::unordered_map<int, AbstractObserver *> observers;
    std::unordered_map<int, AbstractObserver *> mapped_clients;

    bool running = false;

    GrokLoop() : queue(&this->allocator) {

    }

    static IPv4 accept4(int server_fd) {
        sockaddr_in caddr{};
        socklen_t len = sizeof(caddr);

        int client_fd = ::accept(server_fd, reinterpret_cast<sockaddr *>(&caddr),
                                 &len);

        return IPv4(caddr, client_fd);
    }

    void run();
};

#endif //GROKLOOP_EVENT_LOOP_HPP
