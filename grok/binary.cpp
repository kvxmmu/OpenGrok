//
// Created by kvxmmu on 4/27/20.
//

#include "binary.hpp"

BinaryWriter::BinaryWriter(char *_buffer) : buffer(_buffer) {

}

void BinaryWriter::write(char *data, size_t length) {
    memcpy(this->buffer+this->offset, data, length);
    this->offset += length;
}

