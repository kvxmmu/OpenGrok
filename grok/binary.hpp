//
// Created by kvxmmu on 4/27/20.
//

#ifndef OPENGROK_BINARY_HPP
#define OPENGROK_BINARY_HPP

#include <cstring>


template <typename T>
void write_data(char *buffer, const T &data, size_t offset) {
    memmove(buffer+offset, &data, sizeof(T));
}

template <typename T>
T read_data(char *buffer, size_t offset) {
    T out;
    memmove(buffer+offset, &out, sizeof(T));
    return out;
}


class BinaryWriter {
public:
    char *buffer;
    size_t offset = 0;

    explicit BinaryWriter(char *_buffer);

    template <typename T>
    void write(const T &data) {
        write_data<T>(this->buffer, data, this->offset);
        this->offset += sizeof(T);
    }

    void write(char *data, size_t length);
};

class BinaryReader {
public:
    char *buffer;
    size_t offset = 0;

    explicit BinaryReader(char *_buff) : buffer(_buff) {}

    template <typename T>
    T read() {
        T out{};
        memmove(&out, this->buffer+this->offset, sizeof(T));
        this->offset += sizeof(T);
        return out;
    }
};


#endif //OPENGROK_BINARY_HPP
