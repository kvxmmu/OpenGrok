//
// Created by kvxmmu on 5/1/20.
//

#ifndef OPENGROK_BIN_HPP
#define OPENGROK_BIN_HPP

#include <cstring>

template <size_t buffer_size>
class BinaryWriter {
public:
    char buffer[buffer_size]{};
    size_t offset = 0;

    template <typename T>
    void write(const T &data) {
        memmove(this->buffer+this->offset, &data, sizeof(T));
        this->offset += sizeof(T);
    }
};

class BinaryBufferWriter {
public:
    size_t offset = 0;
    char *buffer;

    explicit BinaryBufferWriter(char *_buff);

    template <typename T>
    void write(const T &data) {
        memmove(this->buffer+this->offset, &data, sizeof(T));
        this->offset += sizeof(T);
    }
};

class BinaryReader {
public:
    char *buffer{};
    size_t offset = 0;

    explicit BinaryReader(char *_buff);

    template <typename T>
    T read() {
        T out{};
        memcpy(&out, this->buffer+this->offset, sizeof(T));
        this->offset += sizeof(T);
        return out;
    }
};

#endif //OPENGROK_BIN_HPP
