//
// Created by nero on 12.06.2021.
//

#ifndef GROKPP_PROTO_SPEC_H
#define GROKPP_PROTO_SPEC_H

#include <cinttypes>

#define GROK_COMPRESSED_PKT 0b01
#define GROK_SHORT_PKT 0b10

#define GROK_STORE_FLAGS(flags, n) ((n << 2u) | flags)
#define GROK_GET_FLAGS(n) ((n) & 0b11)
#define GROK_IS_SHORT(n) (GROK_GET_FLAGS(n) & GROK_SHORT_PKT)
#define GROK_IS_COMPRESSED(n) (GROK_GET_FLAGS(n) & GROK_COMPRESSED_PKT)
#define GROK_GET_TYPE(n) ((n) >> 2u)


#define GROK_PING 0
#define GROK_ERROR 1
#define GROK_CREATE_SERVER 2
#define GROK_PACKET 3
#define GROK_CONNECTED 4
#define GROK_DISCONNECT 5

#define GROK_STAT 6

//// Errors

#define NO_SUCH_COMMAND 0
#define NO_SESSION 1
#define TOO_SHORT_BUFFER 2
#define TOO_LONG_BUFFER 3
#define SERVER_ALREADY_CREATED 4
#define BUFFER_SIZE_INCORRECT 5
#define NO_SUCH_CLIENT 6

#define INTERNAL_SERVER_ERROR 255
#define NOT_IMPLEMENTED 128

//// Types

typedef uint32_t c_id_t;

#endif //GROKPP_PROTO_SPEC_H
