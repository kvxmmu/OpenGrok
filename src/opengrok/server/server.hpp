//
// Created by kvxmmu on 3/27/21.
//

#ifndef OPENGROK_SERVER_HPP
#define OPENGROK_SERVER_HPP

#include <opengrok/protocol_definition.hpp>
#include <opengrok/server/interface.hpp>

#include <cameleo/loop.hpp>
#include <cameleo/abstract.hpp>

#include <grokbuffer/wrapper.hpp>

namespace OpenGrok {
    using namespace Cameleo;

    class MainServer {
    public:
         EventLoop &loop;
         GrokBufferProtocol::Server &protocol;

         typedef GrokBufferProtocol::Server::type_t type_t;
         typedef GrokBufferProtocol::Server::length_t length_t;

         MainServer(EventLoop &_loop,
                 GrokBufferProtocol::Server &_protocol) : loop(_loop), protocol(_protocol) {

         }

         void on_packet(BufferReader &reader, int fd,
                 type_t type, length_t length);
         void on_disconnect(int fd);
    };
}

#endif //OPENGROK_SERVER_HPP
