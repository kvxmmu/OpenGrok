//
// Created by nero on 12.06.2021.
//

#ifndef GROKPP_STREAMER_HPP
#define GROKPP_STREAMER_HPP

#include <loop/bytebuffer.hpp>
#include <loop/loop.hpp>
#include "proto_spec.h"

class FStreamer : public BufferStreamer {
public:
    FStreamer(Loop *_loop, sock_t _target,
              char *_data, Endianess _endianess = Endianess::LITTLE) : BufferStreamer(_loop, _target, _data, _endianess) {

    }

    void send_header(uint8_t type, uint32_t length) {
        if (length <= 0xffu) {
            unsigned char buf[sizeof(uint8_t)+sizeof(uint8_t)];
            buf[0] = GROK_STORE_FLAGS(GROK_SHORT_PKT, type);
            buf[1] = static_cast<uint8_t>(length);

            this->send(reinterpret_cast<char *>(buf),
                       sizeof(uint8_t)+sizeof(uint8_t));

            return;
        }


        char buf[sizeof(uint8_t)+sizeof(uint32_t)];
        int_to_bytes<uint8_t>(GROK_STORE_FLAGS(0, type), buf, this->endianess);
        int_to_bytes(length, buf+1, this->endianess);

        this->send(buf, sizeof(uint8_t)+sizeof(uint32_t));
    }

    void send_error(uint8_t type) {
        this->send_header(GROK_ERROR, 1);
        this->send<uint8_t>(type);
    }
};


#endif //GROKPP_STREAMER_HPP
