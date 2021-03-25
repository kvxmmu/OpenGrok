//
// Created by kvxmmu on 3/25/21.
//

#ifndef GROKLOOP_INTERFACES_HPP
#define GROKLOOP_INTERFACES_HPP

class IProxyObserver {
public:
    virtual void forward(char *buffer, uint32_t length) = 0;
    virtual void force_disconnect() = 0;
};

class IProxy {
public:
    virtual void forward(uint64_t client_id, char *buffer, uint32_t length) = 0;
    virtual void disconnect(uint64_t client_id) = 0;
};

#endif //GROKLOOP_INTERFACES_HPP
