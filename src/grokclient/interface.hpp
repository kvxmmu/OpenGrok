//
// Created by nero on 13.06.2021.
//

#ifndef GROKPP_INTERFACE_HPP
#define GROKPP_INTERFACE_HPP

#include <freegrok/proto_spec.h>

class IClient {
public:
    virtual void redirect_packet(c_id_t client_id, char *buffer,
                                 size_t length) = 0;
    virtual void send_disconnected(c_id_t client_id) = 0;
};


#endif //GROKPP_INTERFACE_HPP
