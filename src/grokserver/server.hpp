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


class User {
public:
    enum Rights {
        CAN_SELECT_PORT,
        ALL
    };

    GrokProxy *server = nullptr;
    bool can_select_port = false;

    constexpr void grant(Rights rights) {
        switch (rights) {
            case CAN_SELECT_PORT: {
                can_select_port = true;

                break;
            }

            case ALL:{
                can_select_port = true;

                break;
            }
        }
    }

    [[nodiscard]]
    constexpr bool has_rights(Rights rights) const noexcept {
        switch (rights) {
            case CAN_SELECT_PORT:
                return can_select_port;

            case ALL:
                return false;
        }
    }
};



class FreeGrok : public IObserver, public IMainServer {
private:
    std::unordered_map<sock_t, User> users;

    enum PacketState {
        TYPE_GOT,
        HEADER_GOT,
        BUFFER_GOT
    };

protected:
    void process_message(uint8_t type, uint32_t length,
                         FStreamer &streamer);

    User &gather_user(sock_t fd);
    bool has_user(sock_t fd);
    bool has_server(sock_t fd);
    void link_server(sock_t fd, GrokProxy *proxy);
    void remove_user(sock_t fd);
    void remove_server(sock_t fd);
    GrokProxy *get_server(sock_t fd);

public:
    Config cfg;
    char *magic_auth_buffer;
    size_t magic_length;

    explicit
    FreeGrok(Config _cfg,
             char *_magic_auth, size_t _len) : cfg(std::move(_cfg)), IObserver(tcp_create(), true),
                                               magic_auth_buffer(_magic_auth), magic_length(_len) {

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
