//
// Created by nero on 12.06.2021.
//

#ifndef GROKPP_IOBSERVER_HPP
#define GROKPP_IOBSERVER_HPP

#include "sockets.h"

#include <unordered_set>

typedef int state_t;
class Loop;
class ReadItem;

class IObserver {
public:
    std::unordered_set<sock_t> clients;

    Loop *loop = nullptr;

    bool is_connected = false;
    bool is_server;
    sock_t socket;

    uint8_t flags;

    IObserver(sock_t _socket, bool _is_server,
              uint8_t _flags = 0) : is_server(_is_server), socket(_socket), flags(_flags) {

    }

    virtual void init(Loop *_loop) {
        loop = _loop;
    }

    virtual void post_init() = 0;

    virtual sock_t on_connect() = 0;
    virtual void post_connect(sock_t sock) {}
    virtual void on_received(state_t state, ReadItem &item) = 0;
    virtual void on_disconnect(sock_t sock) = 0;

    virtual ~IObserver() = default;
};

#endif //GROKPP_IOBSERVER_HPP
