//
// Created by nero on 12.06.2021.
//

#ifndef GROKPP_PROXY_CLIENT_HPP
#define GROKPP_PROXY_CLIENT_HPP

#include <iostream>
#include <loop/loop.hpp>
#include <freegrok/streamer.hpp>
#include "interface.hpp"


#define GROK_PROXY_RECV_BUF_SIZE 4096


class GrokProxyClient : public IObserver {
protected:
    c_id_t self_id;
    IClient *client;

    uint32_t connect_host;
    uint16_t connect_port;

    bool shut = false;

public:
    GrokProxyClient(IClient *_client, c_id_t self,
                    uint32_t host, uint16_t port) :  self_id(self), client(_client),
                                                     IObserver(tcp_create(), false),
                                                     connect_host(host), connect_port(port) {

    }

    void post_init() override;

    void redirect(char *buffer, size_t length);
    void on_received(state_t state, ReadItem &item) override;

    sock_t on_connect() override;
    void on_disconnect(sock_t sock) override;

    void shutdown() {
        shut = true;
        std::cout << "[FreeGrok:ProxyClient] Shutting down client " << self_id << std::endl;
    }
};


#endif //GROKPP_PROXY_CLIENT_HPP
