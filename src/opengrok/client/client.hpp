//
// Created by kvxmmu on 3/28/21.
//

#ifndef OPENGROK_CLIENT_HPP
#define OPENGROK_CLIENT_HPP


#include <opengrok/protocol_definition.hpp>
#include <opengrok/client/interface.hpp>
#include <opengrok/client/proxy_client.hpp>

#include <cameleo/loop.hpp>
#include <cameleo/abstract.hpp>

#include <grokbuffer/wrapper.hpp>

using namespace Cameleo;

namespace OpenGrok {
    class Client : public IClient {
    public:
        const char *ip;
        uint16_t port;

        EventLoop &loop;

        GrokBufferProtocol::Client &protocol;

        typedef GrokBufferProtocol::Client::type_t type_t;
        typedef GrokBufferProtocol::Client::length_t length_t;

        std::unordered_map<client_id_t, IProxyClient *> clients;

        Client(const char *_ip,
                uint16_t _port,
                EventLoop &_loop,
                GrokBufferProtocol::Client &_protocol) : ip(_ip), port(_port), loop(_loop), protocol(_protocol) {

        }

        void on_packet(BufferReader &reader, int fd,
                type_t type, length_t length);
        void on_motd(int fd);
        void on_disconnect(int fd);

        void create_server();
        void get_server_name();

        //// IClient

        void forward(client_id_t client_id, char *buffer, size_t length) override;
        void disconnect(client_id_t client_id) override;
    };
}

#endif //OPENGROK_CLIENT_HPP
