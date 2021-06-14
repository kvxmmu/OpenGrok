//
// Created by nero on 12.06.2021.
//

#ifndef GROKPP_CLIENT_HPP
#define GROKPP_CLIENT_HPP

#include <loop/loop.hpp>
#include <freegrok/proto_spec.h>
#include <freegrok/streamer.hpp>
#include <iostream>

#include "proxy_client.hpp"


class GrokClient : public IObserver, public IClient {
private:
    enum PacketState {
        TYPE_GOT,
        HEADER_GOT,
        BUFFER_GOT
    };

    std::unordered_map<c_id_t, GrokProxyClient *> proxies;

protected:
    uint32_t grok_host;
    uint16_t grok_port;

    uint16_t port;
    uint32_t host;

    void process_message(uint8_t type, uint32_t length,
                         FStreamer &streamer);

public:
    uint16_t listening_port = 0;
    uint32_t listening_host = 0;

    GrokClient(uint32_t _grok_host, uint16_t _grok_port,
               uint32_t _host, uint16_t _port) : grok_host(_grok_host), grok_port(_grok_port),
                                                 host(_host), port(_port), IObserver(tcp_create(), false) {

    }

    void post_init() override {
        std::cout << "[FreeGrok] Connecting to the freegrok server..." << std::endl;

        tcp_connect(socket, grok_host,
                    grok_port);
    }

    sock_t on_connect() override;
    void post_connect(sock_t sock) override;
    void on_received(state_t state, ReadItem &item) override;
    void on_disconnect(sock_t sock) override;

    void go_to_message();

    /// IClient

    void redirect_packet(c_id_t client_id, char *buffer, size_t length) override;
    void send_disconnected(c_id_t client_id) override;
};


#endif //GROKPP_CLIENT_HPP
