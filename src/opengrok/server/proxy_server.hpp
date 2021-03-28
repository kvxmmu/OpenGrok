//
// Created by kvxmmu on 3/28/21.
//

#ifndef OPENGROK_PROXY_SERVER_HPP
#define OPENGROK_PROXY_SERVER_HPP


#include <cameleo/loop.hpp>
#include <cameleo/abstract.hpp>

#include <opengrok/protocol_definition.hpp>
#include <opengrok/server/interface.hpp>

namespace OpenGrok {
    using namespace Cameleo;

    constexpr const static size_t max_proxy_server_listen = 4096;
    constexpr const static size_t chunk_size = 4096;

    class ProxyServer : public IProxyServer, public IObserver {
    public:
        uint16_t listen_port;

        sockaddr_in addr{};
        IMainServer *interface;

        int holder;

        explicit
        ProxyServer(uint16_t _listen_port, IMainServer *_interface,
                int _holder) : IObserver(socket(AF_INET, SOCK_STREAM, 0), true),
                                             listen_port(_listen_port), interface(_interface),
                                             holder(_holder) {
        }

        void init(EventLoop *_loop) override {
            IObserver::init(_loop);

            this->addr.sin_addr.s_addr = INADDR_ANY;
            this->addr.sin_port = htons(this->listen_port);
            this->addr.sin_family = AF_INET;

            Net::set_reuse(this->sockfd);

            dynamic_assert(bind(this->sockfd, reinterpret_cast<sockaddr *>(&this->addr),
                                sizeof(this->addr)) == 0,
                           "Can't bind the port");
            dynamic_assert(listen(this->sockfd, max_proxy_server_listen) == 0,
                           "Can't listen the port");
        }

        int get_listening_port() override {
            sockaddr_in caddr{};
            socklen_t len = sizeof(caddr);

            int status = getsockname(this->sockfd, reinterpret_cast<sockaddr *>(&caddr),
                    &len);

            if (status < 0) {
                perror("getsockname()");
            }

            return ntohs(caddr.sin_port);
        }

        void capturer(Future *future);

        void forward(client_id_t client_id, char *buffer, size_t length) override;
        void disconnect(client_id_t client_id) override;

        int on_connect() override;
        void on_disconnect(int fd) override;

        void shutdown() override;
    };
}

#endif //OPENGROK_PROXY_SERVER_HPP
