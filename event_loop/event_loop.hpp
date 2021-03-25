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
#include "event_loop_errors.hpp"

#define NONSTATIC_CALLBACK(callback) [this](Future *future) { callback(future); }
#define NONSTATIC_CALLBACK1(callback, arg1) [this, arg1](Future *future) { callback(future, arg1); }
#define NONSTATIC_CALLBACK2(callback, arg1, arg2) [this, arg1, arg2](Future *future) { callback(future, arg1, arg2); }

class Future;
class SendItem;

using sendqueue_t = std::deque<SendItem>;
using futures_t = std::deque<Future>;
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
    size_t received = 0;

    int fd;

    future_callback_t callback;
    bool pending;

    Future(FutureType _type, int _fd,
            std::allocator<char> *_allocator,
            future_callback_t _callback) : type(_type), fd(_fd), allocator(_allocator), pending(true), callback(std::move(_callback)) {

    }

    void complete() {
        this->pending = false;
        this->received = 0;
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

    inline bool remove_if_empty(sendqueue_t &queue, int fd) {
        if (queue.empty()) {
            this->queues.erase(fd);

            return true;
        }

        return false;
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

    std::unordered_map<int, futures_t> futures;
    std::unordered_map<int, AbstractObserver *> observers;
    std::unordered_map<int, AbstractObserver *> mapped_clients;

    bool running = false;

    GrokLoop() : queue(&this->allocator) {

    }

    ///// Static Helpers

    static IPv4 accept4(int server_fd) {
        sockaddr_in caddr{};
        socklen_t len = sizeof(caddr);

        int client_fd = ::accept(server_fd, reinterpret_cast<sockaddr *>(&caddr),
                                 &len);

        return IPv4(caddr, client_fd);
    }

    static sockaddr_in connect4(int fd,
            const char *ip, const uint16_t port) {
        sockaddr_in caddr{};
        caddr.sin_port = htons(port);
        caddr.sin_addr.s_addr = inet_addr(ip);
        caddr.sin_family = AF_INET;

        ::connect(fd, reinterpret_cast<sockaddr *>(&caddr), sizeof(caddr));

        return caddr;
    }

    /////

    [[maybe_unused]] void add_observer(AbstractObserver *observer);

    void handle_futures(int fd);
    void remove_futures_if_empty(int fd);

    void map_client(int fd, AbstractObserver *observer);
    void unmap_client(int fd);

    void recv(int fd, size_t count, const future_callback_t &callback);
    void capture(int fd, const future_callback_t &callback);

    void run();
};

#endif //GROKLOOP_EVENT_LOOP_HPP
