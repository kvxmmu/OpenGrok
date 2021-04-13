//
// Created by kvxmmu on 3/27/21.
//

#ifndef OPENGROK_PROTOCOL_DEFINITION_HPP
#define OPENGROK_PROTOCOL_DEFINITION_HPP

#include <opengrok/id_pool.hpp>
#include <cinttypes>

using client_id_t = uint64_t;
using IDPool = ConsistentIDPool<client_id_t>;

//// Packet types

#define ERROR 0u
#define CREATE_SERVER 1u
#define SEND_PACKET 2u
#define CLIENT_CONNECTED 3u
#define CLIENT_DISCONNECTED 4u
#define GET_SERVER_NAME 5u

//// Error types

#define TOO_LONG_PACKET 1u
#define NO_SUCH_COMMAND 2u
#define TOO_SHORT_PACKET 3u
#define TOO_SHORT_SEND_BUFFER 4u
#define NO_SUCH_CLIENT 5u
#define UNINITIALIZED 6u

//// Error descriptions

#define TOO_LONG_PACKET_E "Too long packet size"
#define NO_SUCH_COMMAND_E "No such command"
#define TOO_SHORT_PACKET_E "Too short packet"
#define TOO_SHORT_SEND_BUFFER_E "Too short send buffer"
#define NO_SUCH_CLIENT_E "No such client"
#define UNINITIALIZED_E "Uninitialized session"

////

#endif //OPENGROK_PROTOCOL_DEFINITION_HPP
