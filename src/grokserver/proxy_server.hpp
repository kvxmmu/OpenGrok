//
// Created by nero on 12.06.2021.
//

#ifndef GROKPP_PROXY_SERVER_HPP
#define GROKPP_PROXY_SERVER_HPP

#include <loop/loop.hpp>
#include <iostream>
#include <unordered_map>
#include "interface.hpp"

#define PROXY_PORT 0
#define REDIR_BUF_SIZE 4096


class GrokProxy : public IObserver {
protected:
    sock_t initiator;
    sockaddr_in initiator_peer{};

    std::unordered_map<c_id_t, sock_t> id_to_sock;
    std::unordered_map<sock_t, c_id_t> sock_to_id;

    c_id_t link_client(sock_t sock);
    void unlink_client(c_id_t id, sock_t sock,
                       bool erase = true);

public:
    IMainServer *server;
    uint16_t listening_port;

    GrokProxy(IMainServer *_server,
              sock_t _initiator, uint16_t port = PROXY_PORT) : server(_server), initiator(_initiator), IObserver(tcp_create(), true),
                                                               listening_port(port) {
        tcp_get_peer_name(_initiator, initiator_peer);
    }

    void post_init() override;

    sock_t on_connect() override;
    void on_received(state_t state, ReadItem &item) override;
    void on_disconnect(sock_t sock) override;

    bool has_client(c_id_t id);
    void redirect_packet(c_id_t client_id, char *buffer, size_t length);
    void disconnect_client(c_id_t client_id);
    void shutdown();
};


#endif //GROKPP_PROXY_SERVER_HPP
