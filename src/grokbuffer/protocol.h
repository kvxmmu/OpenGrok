//
// Created by nero on 5/3/2021.
//

#ifndef OPENGROK_PROTOCOL_H
#define OPENGROK_PROTOCOL_H

#include <inttypes.h>
#include <stdbool.h>

typedef uint8_t  type_t;
typedef uint32_t length_t;

typedef struct {
    type_t   packet_type;
    length_t packet_length;

    char *packet;

    bool pending; // little endian only
} GBufPacket;

static void gbuf_init(GBufPacket *packet) {
    packet->packet = NULL;
    packet->packet_length = 0;
    packet->packet_type = 0;
    packet->pending = true;
}

static void gbuf_pending(GBufPacket *packet, bool pending) {
    packet->pending = pending;
}

static void gbuf_type(GBufPacket *packet, type_t type) {
    packet->packet_type = type;
}

static void gbuf_length(GBufPacket *packet, length_t length) {
    packet->packet_length = length;
}

static uint32_t gbuf_u32(void *_data) {
    unsigned char *data = (unsigned char *)_data;

    return ((uint32_t)data[3] << 24u) | ((uint32_t)data[2] << 16u) | ((uint32_t)data[1] << 8u)
    | ((uint32_t)data[0] << 0u);
}

static void gbuf_parsemeta(GBufPacket *packet, char *_meta) {
    unsigned char *meta = (unsigned char *)_meta;

    gbuf_type(packet, meta[0]);
    meta++;

    gbuf_length(packet, gbuf_u32(meta));
}

static uint16_t gbuf_u16(void *_data) {
    unsigned char *data = (unsigned char *)_data;

    return ((uint16_t)data[1] << 8u) | (uint16_t)data[0];
}

static uint64_t gbuf_u64(void *_data) {
    unsigned char *data = (unsigned char *)_data;
    uint32_t first_half = gbuf_u32(data);
    uint32_t last_half = gbuf_u32(data+4u);

    return ((uint64_t)last_half << 32u) | first_half;
}

#endif //OPENGROK_PROTOCOL_H
