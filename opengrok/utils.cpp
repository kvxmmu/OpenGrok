//
// Created by kvxmmu on 5/4/20.
//

#include "utils.hpp"

BinaryBufferWriter::BinaryBufferWriter(char *buffer) : buff(buffer) {

}

template<typename T>
void BinaryBufferWriter::write(const T &data) {
    memmove(this->buff+this->offset, &data, sizeof(T));

    this->offset += sizeof(T);
}

void BinaryBufferWriter::write(char *data, size_t length) {
    memmove(this->buff+this->offset, data, length);

    this->offset += length;
}

BinaryReader::BinaryReader(char *buffer) : buff(buffer) {

}

template<typename T>
T BinaryReader::read() {
    T out{};

    memcpy(&out, this->buff+this->offset, sizeof(T));
    this->offset += sizeof(T);

    return out;
}

char *BinaryReader::read_data(size_t length) {
    char *out = this->buff+this->offset;
    this->offset += length;

    return out;
}

void copy_connect(char *buff, int who) {
    BinaryBufferWriter writer(buff);
    writer.write(CONNECT);
    writer.write(who);
}

void copy_disconnect(char *buff, int who) {
    BinaryBufferWriter writer(buff);
    writer.write(DISCONNECT);
    writer.write(who);
}

void copy_packet(char *buff, int who, int length, char *packet) {
    BinaryBufferWriter writer(buff);
    writer.write(WRITE);
    writer.write(who);
    writer.write(length);
    memcpy(buff+writer.offset, packet, length);
}

void copy_port(char *buff, unsigned short port) {
    BinaryBufferWriter writer(buff);
    writer.write(SEND_PORT);
    writer.write(port);
}
