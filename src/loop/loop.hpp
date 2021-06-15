//
// Created by nero on 12.06.2021.
//

#ifndef GROKPP_LOOP_HPP
#define GROKPP_LOOP_HPP

#include "iobserver.hpp"
#include "selector.hpp"
#include "loop_exceptions.hpp"

#include <unordered_map>
#include <map>
#include <deque>

#define EVENTS_R EPOLLIN | EPOLLHUP | EPOLLRDHUP
#define EVENT_WRITE EPOLLOUT


class WriteItem {
public:
    char *buffer;
    size_t length;
    size_t sent;

    WriteItem(const char *src, size_t _length) : buffer(new char[_length]), length(_length), sent(0) {
        memcpy(buffer, src, _length);
    }

    WriteItem(const WriteItem &) = default;
};

class ReadItem {
public:
    enum Type {
        CAPTURE,
        READ
    };

    Type type;
    sock_t fd;

    state_t state;
    bool pending = true;

    char *buffer;
    size_t received;
    size_t length;

    uint64_t u64 = 0;

    ReadItem(size_t _length, sock_t _fd,
             state_t _state, uint64_t _u64 = 0) : state(_state), length(_length), fd(_fd),
                               type(READ), buffer(new char[_length]),
                               received(0), u64(_u64) {

    }

    ReadItem(sock_t _fd, state_t _state) : state(_state), fd(_fd),
                                           type(CAPTURE), buffer(nullptr),
                                           length(0), received(0) {

    }

    ReadItem(const ReadItem &) = default;
};


class Queues {
public:
    std::deque<WriteItem> write;
    std::deque<ReadItem> read;

    void clear_all() {
        for (auto &w : write) {
            delete[] w.buffer;
        }

        for (auto &r : read) {
            delete[] r.buffer;
        }

        write.clear();
        read.clear();
    }
};


class Loop {
private:
    std::unordered_map<sock_t, Queues> queues;
    std::map<sock_t, IObserver *> observers;
    std::unordered_map<sock_t, IObserver *> linked_clients;

    Selector selector;

protected:
    bool running = false;

    void link_client(sock_t sock, IObserver *observer);
    void unlink_client(sock_t sock, IObserver *observer,
                       bool erase);

    void perform_write_queue(sock_t sock);
    void perform_read_queue(sock_t sock, IObserver *observer);
    void remove_queues(sock_t sock);

public:
    Queues &gather_queue(sock_t fd);

    void add_observer(IObserver *observer);
    void remove_observer(IObserver *observer);

    /// IO

    void capture(sock_t sock, state_t state = -1);
    void recv(sock_t sock, size_t length, state_t state,
              uint64_t u64 = 0);
    void send(sock_t sock, char *buffer, size_t length);

    ////

    void force_disconnect(sock_t client, bool erase = true);
    void stop();

    void run();
};


#endif //GROKPP_LOOP_HPP
