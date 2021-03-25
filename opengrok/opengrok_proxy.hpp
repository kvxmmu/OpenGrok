//
// Created by kvxmmu on 3/25/21.
//

#ifndef GROKLOOP_OPENGROK_PROXY_HPP
#define GROKLOOP_OPENGROK_PROXY_HPP

#include <event_loop.hpp>
#include <abstract.hpp>

#include "interfaces.hpp"
#include "glog.hpp"

#define RECV_CHUNK_SIZE 4096

namespace OpenGrok {
    class ProxyServer : public IProxyServer, public AbstractObserver {
    public:
        uint16_t port;
        sockaddr_in addr{};
        GLog::Logger logger = GLog::Logger("OpenGrok:ProxyServer");

        int proxier_fd;

        explicit ProxyServer(int proxier,
                uint16_t _port, IServer *_server) : proxier_fd(proxier), IProxyServer(_server), AbstractObserver(true,
                socket(AF_INET, SOCK_STREAM, 0)), port(_port) {
            this->addr.sin_addr.s_addr = INADDR_ANY;
            this->addr.sin_port = htons(this->port);
            this->addr.sin_family = AF_INET;

            set_reuse(this->sockfd);

            dynamic_assert(bind(this->sockfd, reinterpret_cast<sockaddr *>(&this->addr),
                    sizeof(this->addr)) >= 0, "Can't bind ProxyServer port");
            dynamic_assert(listen(this->sockfd, MAX_EPOLL_SIZE) >= 0,
                    "Can't listen for ProxyServer port");

            this->logger.add_stdout_streamer();

            this->logger << "Started ProxyServer at port " << _port << GLog::endl;
        }

        void capturer(Future *future);

        int on_connect() override;
        void on_disconnect(int fd) override;

        void forward(GIDPool::value_type client_id, char *buffer, uint32_t length) override;
        void disconnect(GIDPool::value_type client_id) override;
    };
}

#endif //GROKLOOP_OPENGROK_PROXY_HPP
