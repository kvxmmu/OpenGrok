//
// Created by kvxmmu on 8/24/20.
//

#ifndef THINPAK_BYTEBUFFER_HPP
#define THINPAK_BYTEBUFFER_HPP

#include <cstring>


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
        size_t length = sizeof(IntegerType);
        unsigned long long integer;

        memcpy(&integer, &_integer, sizeof(_integer));
        memset(_buffer, 0, sizeof(IntegerType));

        if (integer == 0) {
            _buffer[0] = 0;

            return;
        }

        auto buffer = reinterpret_cast<unsigned char *>(_buffer);
        size_t pos = 0;

        if (endianess == Endianess::LITTLE) {
            while (integer != 0) {
                uint8_t data = integer & 0xffu;
                integer >>= 8u;

                buffer[pos++] = data;
            }
        } else {
            size_t bits = length * 8 - 8;

            while (bits != 0) {
                uint32_t data = (integer >> bits) & 0xffu;
                bits -= 8;

                buffer[pos++] = data;
            }
        }
    }

    template<typename IntegerType>
    IntegerType bytes_to_int(char *_buffer, Endianess endianess = Endianess::LITTLE) {
        auto buffer = reinterpret_cast<unsigned char *>(_buffer);
        IntegerType integer = 0;
        size_t size_length = sizeof(IntegerType);
        size_t pos = 0;

        if (size_length == 1) {
            return buffer[0];
        }

        if (endianess == Endianess::LITTLE) {
            integer = buffer[size_length - 1];
            pos = size_length - 1;

            while (true) {

                integer <<= 8u;
                integer |= buffer[--pos];

                if (pos == 0) {
                    break;
                }
            }
        } else {
            integer = buffer[pos++];

            while (pos < size_length) {
                integer <<= 8u;
                integer |= buffer[pos++];
            }
        }

        return integer;
    }
}

class BufferReader {
public:
    char *buffer;
    size_t offset = 0;

    Endianess endianess;

    explicit BufferReader(char *_buffer,
            Endianess _endianess) : buffer(_buffer), endianess(_endianess) {

    }

    template <typename T>
    T read() {
        T value = bytes_to_int<T>(this->buffer+this->offset, endianess);
        this->offset += sizeof(T);

        return value;
    }
};

template <size_t buffer_size>
class BufferWriter {
public:
    char buffer[buffer_size]{0};
    size_t offset = 0;

    Endianess endianess;

    explicit BufferWriter(Endianess _endianess = Endianess::LITTLE) : endianess(_endianess) {

    }

    template <typename T>
    void write(const T &value) {
        int_to_bytes(value, this->buffer+this->offset, this->endianess);

        this->offset += sizeof(T);
    }
};

class DynamicBufferWriter {
public:
    char *buffer = nullptr;

    const static size_t default_capacity = 8;

    size_t capacity = default_capacity;
    size_t end = 0;

    Endianess endianess;

    DynamicBufferWriter(Endianess _endianess = Endianess::LITTLE) : buffer(new char[default_capacity]), endianess(_endianess) {

    }

    void write(char *src, size_t count) {
        if (this->end+count < this->capacity) {
            auto new_buffer = new char[this->capacity << 1u];
            memcpy(new_buffer, this->buffer, this->end);

            delete[] this->buffer;

            this->buffer = new_buffer;
        }

        memcpy(this->buffer+this->end, src, count);
        this->end += count;
    }

    template <typename __ssize_t = uint8_t>
    void write_string(const std::string &str) {
        char *strbuff = new char[str.size() + sizeof(__ssize_t)];
        int_to_bytes<__ssize_t>(str.size(), strbuff);

        memcpy(strbuff+sizeof(__ssize_t), str.c_str(), str.size());

        this->write(strbuff, str.size() + sizeof(__ssize_t));

        delete[] strbuff;
    }

    template <typename Integral>
    void write(Integral integral) {
        char integral_buffer[sizeof(Integral)];

        int_to_bytes<Integral>(integral, integral_buffer);

        this->write(integral_buffer, sizeof(Integral));
    }

    ~DynamicBufferWriter() {
        delete[] this->buffer;
    }
};

#endif //THINPAK_BYTEBUFFER_HPP
