//
// Created by kvxmmu on 3/25/21.
//

#ifndef GROKLOOP_CLIENT_OBSERVER_HPP
#define GROKLOOP_CLIENT_OBSERVER_HPP

#include <event_loop.hpp>
#include <abstract.hpp>

#include "../opengrok/glog.hpp"
#include "../opengrok/types.h"

#include "interfaces.hpp"
#include "client_proxy.hpp"

class ClientObserver : public IProxy, public AbstractObserver {
public:
    GLog::Logger logger = GLog::Logger("OpenGrok:ClientObserver");

    sockaddr_in caddr{};

    uint16_t relay_port;
    const char *relay_ip;

    ClientObserver(uint16_t server_port,
            const char *server_ip, const char *_relay_ip,
            uint16_t _relay_port) : AbstractObserver(false, socket(AF_INET, SOCK_STREAM, 0)),
                                    relay_ip(_relay_ip), relay_port(_relay_port) {
        this->logger.add_stdout_streamer();

        this->caddr.sin_addr.s_addr = inet_addr(server_ip);
        this->caddr.sin_port = htons(server_port);
        this->caddr.sin_family = AF_INET;
    }

    void create_server() {
        BufferWriter<sizeof(uint8_t)> writer;

        writer.write<uint8_t>(CREATE_SERVER);

        this->loop->send(this->sockfd, writer.buffer, writer.offset);
    }

    void type_getter(Future *future);
    void packet_getter(Future *future, uint64_t client_id, uint32_t length);

    void fallback_to_type() {
        this->loop->recv(this->sockfd, sizeof(uint8_t), NONSTATIC_CALLBACK(this->type_getter));
    }

    void post_init() override {
        ::connect(this->sockfd, reinterpret_cast<sockaddr *>(&this->caddr),
                sizeof(this->caddr));

        this->create_server();
    }

    int on_connect() override;
    void on_disconnect(int fd) override;

    void forward(uint64_t client_id, char *buffer, uint32_t length) override;
    void disconnect(uint64_t client_id) override;
};


#endif //GROKLOOP_CLIENT_OBSERVER_HPP
