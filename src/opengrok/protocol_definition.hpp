//
// Created by kvxmmu on 3/27/21.
//

#ifndef OPENGROK_PROTOCOL_DEFINITION_HPP
#define OPENGROK_PROTOCOL_DEFINITION_HPP

#include <opengrok/id_pool.hpp>
#include <cinttypes>

using IDPool = ConsistentIDPool<uint64_t>();

//// Packet types

#define ERROR 0u
#define CREATE_SERVER 1u

//// Error types

#define TOO_LONG_PACKET 1u

//// Error descriptions

#define TOO_LONG_PACKET_E "Too long packet size"

////

#endif //OPENGROK_PROTOCOL_DEFINITION_HPP
