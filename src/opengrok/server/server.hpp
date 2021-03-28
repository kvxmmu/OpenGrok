//
// Created by kvxmmu on 3/27/21.
//

#ifndef OPENGROK_SERVER_HPP
#define OPENGROK_SERVER_HPP

#include <opengrok/protocol_definition.hpp>
#include <opengrok/server/interface.hpp>
#include <opengrok/server/proxy_server.hpp>

#include <cameleo/loop.hpp>
#include <cameleo/abstract.hpp>


#include <grokbuffer/wrapper.hpp>

namespace OpenGrok {
    using namespace Cameleo;

    class MainServer : public IMainServer {
    public:
         EventLoop &loop;
         GrokBufferProtocol::Server &protocol;

         std::string app_name;

         typedef GrokBufferProtocol::Server::type_t type_t;
         typedef GrokBufferProtocol::Server::length_t length_t;

         std::unordered_map<int, IProxyServer *> servers;
         std::allocator<char> local_allocator;

         MainServer(EventLoop &_loop,
                 GrokBufferProtocol::Server &_protocol,
                 std::string _app_name) : loop(_loop), protocol(_protocol), app_name(std::move(_app_name)) {

         }

         void on_packet(BufferReader &reader, int fd,
                 type_t type, length_t length);
         void on_disconnect(int fd);

         //// IMainServer

         void connect(int dest, client_id_t client_id) override;
         void forward(int dest, client_id_t client_id, char *buffer, size_t length) override;
         void disconnect(int dest, client_id_t client_id) override;

         //// Responders

         void respond_port(int dest, uint16_t port);

         ////
    };
}

#endif //OPENGROK_SERVER_HPP
