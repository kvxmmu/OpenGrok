//
// Created by kvxmmu on 3/27/21.
//

#ifndef CAMELEO_LOOP_HPP
#define CAMELEO_LOOP_HPP

#include <cameleo/abstract.hpp>
#include <cameleo/selector.hpp>

#include <cameleo/net_helpers.h>
#include <cameleo/net.h>

#include <memory>
#include <functional>
#include <deque>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <stdexcept>
#include <cstring>

#include <unordered_map>

#define NONSTATIC_CALLBACK(callback) [this](::Cameleo::Future *future) { callback(future); }
#define NONSTATIC_CALLBACK1(callback, arg1) [this, arg1](::Cameleo::Future *future) { callback(future, arg1); }
#define NONSTATIC_CALLBACK2(callback, arg1, arg2) [this, arg1, arg2](::Cameleo::Future *future) { callback(future, arg1, arg2); }

namespace Cameleo {
#ifndef CAMELEO_OWN_ALLOCATOR
    typedef std::allocator<char> cmle_allocator_t;
#endif

    class SendItem {
    public:
        char *write_buffer = nullptr;
        size_t capacity = 0;
        size_t written;

        SendItem(char *buffer, size_t cap) : write_buffer(buffer), capacity(cap), written(0) {

        }

        void deallocate(cmle_allocator_t *_allocator) {
            _allocator->deallocate(this->write_buffer, this->capacity);
        }
    };

    class SendQueue {
    public:
        typedef std::deque<SendItem> items_t;

        cmle_allocator_t *allocator;
        std::unordered_map<int, items_t> queues;

        explicit SendQueue(cmle_allocator_t *_allocator) : allocator(_allocator) {

        }

        inline bool has_queue(int fd) {
            return this->queues.find(fd) != this->queues.end();
        }

        std::pair<bool, bool> perform(int fd) {
            if (!this->has_queue(fd) || this->queues.at(fd).empty()) {
                throw std::logic_error("No queue for this fd");
            }

            bool need_remove = false;
            bool error_occur = false;
            auto &queue = this->queues[fd];
            auto &front = queue.front();

            int bytes_written = Net::write_bytes(fd, front.write_buffer+front.written,
                                            front.capacity - front.written);

            if (bytes_written <= 0) {
                front.written = front.capacity;
                error_occur = true;
            } else {
                front.written += bytes_written;
            }

            if (front.written >= front.capacity) {
                front.deallocate(this->allocator);
                queue.pop_front();
            }

            need_remove = queue.empty();
            this->remove_if_empty(fd);

            return std::make_pair(need_remove, error_occur);
        }

        void push(int fd, char *buffer, size_t length) {
            auto alloc_buffer = this->allocator->allocate(length);
            memcpy(alloc_buffer, buffer, length);

            SendItem item(alloc_buffer, length);

            this->create_if_empty(fd);
            this->queues[fd].push_back(item);
        }

        void create_if_empty(int fd) {
            if (!this->has_queue(fd)) {
                this->queues[fd] = items_t();
            }
        }

        void remove_if_empty(int fd) {
            if (!this->has_queue(fd)) {
                return;
            }

            if (this->queues[fd].empty()) {
                this->queues.erase(fd);
            }
        }

        void clear_queue(int fd) {
            if (!this->has_queue(fd)) {
                return;
            }

            for (auto &item : this->queues.at(fd)) {
                item.deallocate(this->allocator);
            }

            this->queues.erase(fd);
        }
    };

    class Future {
    public:
        typedef std::function<void(Future *)> callback_t;

        enum FutureType {
            READ, CAPTURE
        };

        char *recv_buffer = nullptr;
        size_t capacity = 0;
        size_t received = 0;

        bool pending = true;

        cmle_allocator_t *allocator;

        FutureType type;
        int fd;

        callback_t callback;

        Future(int _fd, FutureType _type,
                cmle_allocator_t *_allocator, callback_t _callback) : fd(_fd), type(_type),
                                                                      allocator(_allocator), callback(std::move(_callback)) {

        }

        void complete() {
            this->pending = false;
            this->received = 0;
        }

        char *allocate(size_t count) {
            this->recv_buffer = this->allocator->allocate(count);
            this->capacity = count;

            return this->recv_buffer;
        }

        void deallocate() {
            this->allocator->deallocate(this->recv_buffer, this->capacity);
            this->capacity = 0;
            this->recv_buffer = nullptr;
        }
    };

    class EventLoop {
    public:
        typedef std::deque<std::shared_ptr<Future>> futures_t;

        EpollSelector selector;
        cmle_allocator_t allocator; // for future's buffers

        std::unordered_map<int, futures_t> futures;
        std::unordered_map<int, IObserver *> observers;

        SendQueue send_queue;

        bool running = false;

        EventLoop() : send_queue(&allocator) {

        }

        //// Futures

        bool futures_exists(int fd);
        void create_futures_if_empty(int fd);

        void send(int fd, char *buffer, size_t length);
        void recv(int fd, size_t count,
                const std::function<void(Future *)> &callback);
        void capture(int fd, const Future::callback_t &capture_func);

        void handle_futures(int fd);
        void clear_futures(int fd);
        void remove_if_empty_fut(int fd);

        ////

        static auto accept4(int fd) -> Client { // hz zachem, pust budet
            sockaddr_in addr{};
            socklen_t len = sizeof(addr);

            int sockfd = ::accept(fd, reinterpret_cast<sockaddr *>(&addr),
                    &len);

            return {sockfd, addr};
        }

        void add_observer(IObserver *observer);
        void remove_observer(IObserver *observer);

        void force_disconnect(int fd);

        void run();
    };
}

#endif //CAMELEO_LOOP_HPP
