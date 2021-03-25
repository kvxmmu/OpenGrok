//
// Created by kvxmmu on 3/25/21.
//

#ifndef GROKLOOP_OPENGROK_HPP
#define GROKLOOP_OPENGROK_HPP

#include <event_loop.hpp>
#include <abstract.hpp>

#include "glog.hpp"
#include "types.h"
#include "interfaces.hpp"
#include "id_pool.hpp"

#include "opengrok_proxy.hpp"

namespace OpenGrok {
    class MainServer : public IServer, public AbstractObserver {
    public:
        GLog::Logger logger = GLog::Logger("OpenGrok:MainServer");
        sockaddr_in addr{};

        uint16_t port;

        explicit MainServer(uint16_t listen_port) : port(listen_port), AbstractObserver(true, socket(AF_INET, SOCK_STREAM, 0)) {
            this->logger.add_stdout_streamer();

            this->logger << "Logger test" << GLog::endl;
        }

        void init(GrokLoop *_loop) override {
            AbstractObserver::init(_loop);

            this->addr.sin_port = htons(this->port);
            this->addr.sin_family = AF_INET;
            this->addr.sin_addr.s_addr = INADDR_ANY;

            set_reuse(this->sockfd);

            dynamic_assert(bind(this->sockfd, reinterpret_cast<sockaddr *>(&this->addr),
                    sizeof(this->addr)) >= 0, "Can't bind port");
            dynamic_assert(listen(this->sockfd, MAX_EPOLL_SIZE) >= 0, "Can't listen to port");
        }

        void fallback_to_type(int fd) {
            this->loop->recv(fd, sizeof(uint8_t), NONSTATIC_CALLBACK(this->type_getter));
        }

        void write_port(int fd, uint16_t lport);
        void write_error(int fd, uint8_t error_code, const char *error_description);

        void disconnect_target_getter(Future *future);
        void packet_getter(Future *future, uint64_t client_id,
                           uint32_t length);
        void type_getter(Future *future);

        int on_connect() override;
        void on_disconnect(int fd) override;

        void connect(int dest, GIDPool::value_type client_id) override;
        void forward(int dest,
                     GIDPool::value_type client_id, char *buffer, uint32_t length) override;
        void disconnect(int dest, GIDPool::value_type client_id) override;
    };
}

#endif //GROKLOOP_OPENGROK_HPP
