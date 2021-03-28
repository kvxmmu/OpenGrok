//
// Created by kvxmmu on 3/28/21.
//

#ifndef OPENGROK_PROXY_CLIENT_HPP
#define OPENGROK_PROXY_CLIENT_HPP


#include <cameleo/loop.hpp>
#include <cameleo/abstract.hpp>

#include <opengrok/client/interface.hpp>
#include <opengrok/protocol_definition.hpp>

using namespace Cameleo;

namespace OpenGrok {
    class ProxyClient : public IObserver, public IProxyClient {
    public:
        sockaddr_in caddr{};

        IClient *interface;
        client_id_t client_id;

        explicit
        ProxyClient(sockaddr_in _addr, client_id_t _client_id,
                IClient *_interface) : IObserver(socket(AF_INET, SOCK_STREAM, 0), false),
                                         caddr(_addr), client_id(_client_id), interface(_interface) {

        }

        void capturer(Future *future);

        void post_init() override {
            ::connect(this->sockfd, reinterpret_cast<sockaddr *>(&this->caddr),
                    sizeof(this->caddr));
        }

        int on_connect() override;
        void on_disconnect(int fd) override;

        void forward(char *buffer, size_t count) override;
    };
}

#endif //OPENGROK_PROXY_CLIENT_HPP
