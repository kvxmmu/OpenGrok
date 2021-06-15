#ifndef THINPAK_BYTEBUFFER_HPP
#define THINPAK_BYTEBUFFER_HPP

#include <cstring>
#include "loop.hpp"


#if defined(__GNUC__) || defined(__llvm__) || defined(__INTEL_COMPILER)
#define GNU_EXTS
#endif

#ifdef GNU_EXTS
#define likely(x)     __builtin_expect(!!(x),1)
#define unlikely(x)   __builtin_expect(!!(x),0)
#else
#define likely(x)     (x)
#define unlikely(x)   (x)
#endif


enum Endianess {
    BIG, LITTLE
};

namespace {
    uint32_t uint32_from_float(float x) {
        uint32_t uint32;

        memcpy(&uint32, &x, sizeof(uint32_t));

        return uint32;
    }

    float float_from_uint32(uint32_t uint32) {
        float x;

        memcpy(&x, &uint32, sizeof(uint32_t));

        return x;
    }

    uint32_t int32_to_uint32(int32_t int32) {
        uint32_t uint32;

        memcpy(&uint32, &int32, sizeof(int32_t));

        return uint32;
    }

    uint16_t int16_to_uint16(int16_t int16) {
        uint16_t uint16;

        memcpy(&uint16, &int16, sizeof(int16_t));

        return uint16;
    }

    int16_t uint16_to_int16(uint16_t uint16) {
        int16_t int16;

        memcpy(&int16, &uint16, sizeof(int16_t));

        return int16;
    }

    template<typename IntegerType>
    void int_to_bytes(IntegerType _integer,
                      char *_buffer, Endianess endianess = Endianess::LITTLE) {
        constexpr const auto int_sz = sizeof(IntegerType);
        auto buffer = reinterpret_cast<unsigned char *>(_buffer);

        if (endianess == Endianess::LITTLE) {
            for (size_t pos = 0; pos < int_sz; pos++) {
                buffer[pos] = _integer & 0xffu;
                _integer >>= 8u;
            }
        } else {
            for (size_t pos = int_sz; pos > 0; pos--) {
                buffer[pos] = _integer & 0xffu;
                _integer >>= 8u;
            }
        }
    }

    template <typename IntegerType>
    IntegerType bytes_to_int(char *_buffer, const Endianess endianess = Endianess::LITTLE) {
        auto buffer = reinterpret_cast<unsigned char *>(_buffer);

        if constexpr (sizeof(IntegerType) == 1) {
            return static_cast<IntegerType>(buffer[0]);
        }

        IntegerType value = buffer[endianess == Endianess::LITTLE ? sizeof(IntegerType)-1 : 0];

        if (endianess == LITTLE) {
            size_t pos = sizeof(IntegerType)-1;

            while (pos != 0) {
                value = (value << 8u) | buffer[--pos];
            }
        } else {
            size_t pos = 1;

            while (pos < sizeof(IntegerType)) {
                value = (value << 8u) | buffer[pos++];
            }
        }

        return value;
    }
}

class BufferReader {
public:
    char *buffer;
    size_t offset = 0;

    Endianess endianess;

    explicit BufferReader(char *_buffer,
                          Endianess _endianess = LITTLE) : buffer(_buffer), endianess(_endianess) {

    }

    template <typename T>
    T read() {
        T value = bytes_to_int<T>(this->buffer+this->offset, endianess);
        this->offset += sizeof(T);

        return value;
    }

    void read_buffer(char *dest, size_t length) {
        memcpy(this->buffer+this->offset, dest, length);

        this->offset += length;
    }

    template <typename string_size_t>
    std::string read_string() {
        auto size = this->read<string_size_t>();
        std::string str(this->buffer+this->offset, size);

        this->offset += size;

        return str;
    }

    char *get_ptr(size_t size) {
        char *ret_buff = this->buffer+this->offset;

        this->offset += size;

        return ret_buff;
    }
};

class BufferStreamer {
public:
    Loop *loop;
    sock_t target;
    Endianess endianess;

    BufferReader reader;

    BufferStreamer(Loop *_loop, sock_t _target,
                   char *data,
                   Endianess _endianess = Endianess::LITTLE) : loop(_loop), target(_target),
                                                               endianess(_endianess), reader(data, _endianess) {

    }

    virtual void send(char *buffer, size_t length) const {
        loop->send(target, buffer, length);
    }
};

#endif //THINPAK_BYTEBUFFER_HPP