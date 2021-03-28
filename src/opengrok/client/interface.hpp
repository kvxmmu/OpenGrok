//
// Created by kvxmmu on 3/28/21.
//

#ifndef OPENGROK_IINTERFACE_HPP
#define OPENGROK_IINTERFACE_HPP

#include <opengrok/protocol_definition.hpp>

class IClient {
public:
    virtual void forward(client_id_t client_id, char *buffer, size_t length) = 0;
    virtual void disconnect(client_id_t client_id) = 0;
};

class IProxyClient {
public:
    virtual void forward(char *buffer, size_t count) = 0;
};

#endif //OPENGROK_IINTERFACE_HPP
