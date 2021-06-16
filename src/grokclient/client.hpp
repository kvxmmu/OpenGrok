//
// Created by nero on 12.06.2021.
//

#ifndef GROKPP_CLIENT_HPP
#define GROKPP_CLIENT_HPP

#include <loop/loop.hpp>
#include <freegrok/proto_spec.h>
#include <freegrok/streamer.hpp>
#include <iostream>
#include <stdexcept>

#include "proxy_client.hpp"


class DecompressionError : public std::runtime_error {
public:
    explicit
    DecompressionError(const char *desc) : std::runtime_error(desc) {

    }
};


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

    uint16_t requested_port;

    void process_message(uint8_t type, uint32_t length,
                         FStreamer &streamer);

public:
    uint16_t listening_port = 0;
    uint32_t listening_host = 0;

    std::string magic;

    GrokClient(uint32_t _grok_host, uint16_t _grok_port,
               uint32_t _host, uint16_t _port,
               std::string _magic, uint16_t _req_port) : grok_host(_grok_host), grok_port(_grok_port), magic(std::move(_magic)),
                                                         host(_host), port(_port), IObserver(tcp_create(), false),
                                                         requested_port(_req_port) {

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
