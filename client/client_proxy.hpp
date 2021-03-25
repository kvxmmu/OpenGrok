//
// Created by kvxmmu on 3/25/21.
//

#ifndef GROKLOOP_CLIENT_PROXY_HPP
#define GROKLOOP_CLIENT_PROXY_HPP

#include <event_loop.hpp>
#include <abstract.hpp>

#include "interfaces.hpp"

#define RECV_CHUNK_SIZE 4096

class ProxyObserver : public IProxyObserver, public AbstractObserver {
public:
    sockaddr_in addr{};
    uint64_t client_id;

    IProxy *proxier;

    ProxyObserver(const char *ip,
            uint16_t port, uint64_t _client_id,
            IProxy *_proxier) : AbstractObserver(false, socket(AF_INET, SOCK_STREAM, 0)),
                                                  client_id(_client_id), proxier(_proxier) {
        this->addr.sin_port = htons(port);
        this->addr.sin_addr.s_addr = inet_addr(ip);
        this->addr.sin_family = AF_INET;
    }

    void post_init() override {
        ::connect(this->sockfd, reinterpret_cast<sockaddr *>(&this->addr),
                sizeof(this->addr));
    }

    void capturer(Future *future);

    int on_connect() override {
        this->is_connected = true;

        return this->sockfd;
    }

    void forward(char *buffer, uint32_t length) override;

    void on_disconnect(int fd) override;
    void force_disconnect() override;
};

#endif //GROKLOOP_CLIENT_PROXY_HPP
