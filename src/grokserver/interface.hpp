//
// Created by nero on 12.06.2021.
//

#ifndef GROKPP_INTERFACE_HPP
#define GROKPP_INTERFACE_HPP

#include "id_pool.hpp"
#include <freegrok/proto_spec.h>

class IMainServer {
public:
    IdPool<c_id_t> id_pool;

    virtual void send_connected(sock_t initiator, c_id_t generated_id) = 0;
    virtual void redirect_packet(sock_t initiator, c_id_t sender,
                                 char *buffer, size_t length) = 0;
    virtual void send_disconnected(sock_t initiator, c_id_t client_id) = 0;
};

#endif //GROKPP_INTERFACE_HPP
