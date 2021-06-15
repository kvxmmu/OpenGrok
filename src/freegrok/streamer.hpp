//
// Created by nero on 12.06.2021.
//

#ifndef GROKPP_STREAMER_HPP
#define GROKPP_STREAMER_HPP

#include <loop/bytebuffer.hpp>
#include <loop/loop.hpp>
#include "proto_spec.h"

#include <zstd.h>
#include <iostream>

#define COMPRESS_MIN_THRESHOLD 100
#define ZSTD_COMPRESS_LEVEL 1

class FStreamer : public BufferStreamer {
public:
    FStreamer(Loop *_loop, sock_t _target,
              char *_data, Endianess _endianess = Endianess::LITTLE) : BufferStreamer(_loop, _target, _data, _endianess) {

    }

    void send_header(uint8_t type, uint32_t length,
                     bool compressed) {
        uint8_t flags = compressed ? GROK_COMPRESSED_PKT : 0;

        if (length <= 0xffu) {
            unsigned char buf[sizeof(uint8_t)+sizeof(uint8_t)];
            buf[0] = GROK_STORE_FLAGS(flags | GROK_SHORT_PKT, type);
            buf[1] = static_cast<uint8_t>(length);

            BufferStreamer::send(reinterpret_cast<char *>(buf),
                       sizeof(uint8_t)+sizeof(uint8_t));

            return;
        }

        char buf[sizeof(uint8_t)+sizeof(uint32_t)];
        int_to_bytes<uint8_t>(GROK_STORE_FLAGS(flags, type), buf, this->endianess);
        int_to_bytes(length, buf+1, this->endianess);

        BufferStreamer::send(buf, sizeof(uint8_t)+sizeof(uint32_t));
    }

    void send(uint8_t type, char *buffer, size_t length) {
        if (length == 0) {
            this->send_header(type, 0, false);

            return;
        }

        char *target_buf = buffer; // at least N bytes to compress
        size_t target_buf_length;
        bool compressed = false;

        if (length >= COMPRESS_MIN_THRESHOLD) {
            target_buf = new char[length];

            target_buf_length = ZSTD_compress(target_buf, length,
                                              buffer, length, ZSTD_COMPRESS_LEVEL);

            if (ZSTD_isError(target_buf_length) || (target_buf_length >= length)) {
                target_buf_length = length;

                delete[] target_buf;
                target_buf = buffer;
            } else {
                length = target_buf_length;
                compressed = true;
            }
        } else {
            target_buf_length = length;
        }

        this->send_header(type, length, compressed);
        BufferStreamer::send(target_buf, target_buf_length);

        if (compressed) delete[] target_buf;
    }

    void send_error(uint8_t type) {
        this->send(GROK_ERROR, reinterpret_cast<char *>(&type), sizeof(uint8_t));
    }
};


#endif //GROKPP_STREAMER_HPP
