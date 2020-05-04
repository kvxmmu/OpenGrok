//
// Created by kvxmmu on 5/4/20.
//

#ifndef OPENGROK2_UTILS_HPP
#define OPENGROK2_UTILS_HPP

#include <cinttypes>
#include <cstring>

#define SEND_PORT static_cast<uint8_t>(1)
#define CONNECT static_cast<uint8_t>(2)
#define WRITE static_cast<uint8_t>(3)
#define DISCONNECT static_cast<uint8_t>(4)

class BinaryBufferWriter {
public:
    size_t offset = 0;
    char *buff;

    explicit BinaryBufferWriter(char *buffer);

    template <typename T>
    void write(const T &data);
    void write(char *data, size_t length);
};


class BinaryReader {
public:
    size_t offset = 0;
    char *buff;

    explicit BinaryReader(char *buffer);

    template <typename T>
    T read();
    char *read_data(size_t length);
};

void copy_connect(char *buff, int who);
void copy_disconnect(char *buff, int who);

void copy_packet(char *buff, int who, int length, char *packet);

void copy_port(char *buff, unsigned short port);


#endif //OPENGROK2_UTILS_HPP
