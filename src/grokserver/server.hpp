//
// Created by nero on 12.06.2021.
//

#ifndef GROKPP_SERVER_HPP
#define GROKPP_SERVER_HPP

#include <iostream>
#include <freegrok/proto_spec.h>
#include <freegrok/streamer.hpp>
#include <loop/loop.hpp>

#include "interface.hpp"
#include "proxy_server.hpp"
#include "config.hpp"


class FreeGrok : public IObserver, public IMainServer {
private:
    std::unordered_map<sock_t, GrokProxy *> servers;

    enum PacketState {
        TYPE_GOT,
        HEADER_GOT,
        BUFFER_GOT
    };

protected:
    void process_message(uint8_t type, uint32_t length,
                         FStreamer &streamer);

public:
    Config cfg;

    explicit
    FreeGrok(Config _cfg) : cfg(std::move(_cfg)), IObserver(tcp_create(), true) {

    }

    void post_init() override;

    sock_t on_connect() override;
    void on_received(state_t state, ReadItem &item) override;
    void on_disconnect(sock_t sock) override;

    void go_new_message(sock_t src);

    //// IMainServer

    void send_connected(sock_t initiator, c_id_t generated_id) override;
    void redirect_packet(sock_t initiator, c_id_t sender, char *buffer, size_t length) override;
    void send_disconnected(sock_t initiator, c_id_t client_id) override;
};


#endif //GROKPP_SERVER_HPP
