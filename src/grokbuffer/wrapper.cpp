//
// Created by kvxmmu on 3/27/21.
//

#include "wrapper.hpp"

void GrokBufferProtocol::Server::init(Cameleo::EventLoop *_loop) {
    dynamic_assert(this->packet_callback != nullptr && this->disconnect_callback != nullptr,
            "Can't start with empty callbacks");

    IObserver::init(_loop);

    this->addr.sin_addr.s_addr = this->listen_addr;
    this->addr.sin_port = htons(this->listen_port);
    this->addr.sin_family = AF_INET;

    Net::set_reuse(this->sockfd);

    dynamic_assert(bind(this->sockfd, reinterpret_cast<sockaddr *>(&this->addr),
            sizeof(this->addr)) == 0,
            "Can't bind the port");
    dynamic_assert(listen(this->sockfd, MAX_LISTEN) == 0,
            "Can't listen the port");
}

int GrokBufferProtocol::Server::on_connect() {
    auto client = Cameleo::EventLoop::accept4(this->sockfd);
    this->fallback_to_type(client.sockfd);

    return client.sockfd;
}

void GrokBufferProtocol::Server::post_connect(int fd) {
    IObserver::post_connect(fd);

    if (this->motd_callback != nullptr) {
        this->motd_callback(fd);
    }
}

void GrokBufferProtocol::Server::on_disconnect(int fd) {
    this->disconnect_callback(fd);
}

//// Getters

void GrokBufferProtocol::Server::type_getter(Cameleo::Future *future) {
    BufferReader reader(future->recv_buffer, this->endianess);
    auto type = reader.read<type_t>();

    this->loop->recv(future->fd, sizeof(length_t), NONSTATIC_CALLBACK1(this->length_getter, type));
}

void GrokBufferProtocol::Server::length_getter(Cameleo::Future *future, GrokBufferProtocol::Server::type_t type) {
    BufferReader reader(future->recv_buffer, this->endianess);
    auto length = reader.read<length_t>();

    if (length == 0u) {
        this->packet_callback(reader, future->fd,
                type, length);
        this->fallback_to_type(future->fd);

        return;
    } else if (length > this->max_packet_length) {
        this->respond_error(future->fd, this->too_long_packet_error_code,
                this->too_long_packet_description);

        this->fallback_to_type(future->fd);

        return;
    }

    this->loop->recv(future->fd, length,
            NONSTATIC_CALLBACK2(this->buffer_getter, type, length));
}

void GrokBufferProtocol::Server::buffer_getter(Cameleo::Future *future, GrokBufferProtocol::Server::type_t type,
                                               GrokBufferProtocol::Server::length_t length) {
    BufferReader reader(future->recv_buffer, this->endianess);

    this->packet_callback(reader, future->fd,
            type, length);
    this->fallback_to_type(future->fd);
}

//// Responders

void GrokBufferProtocol::Server::respond(int dest, GrokBufferProtocol::Server::type_t type, char *buffer,
                                         GrokBufferProtocol::Server::length_t length) {
    BufferWriter<sizeof(type_t)+sizeof(length_t)> writer;
    writer.write<type_t>(type);
    writer.write<length_t>(length);

    this->loop->send(dest, writer.buffer, writer.offset);
    this->loop->send(dest, buffer, length);
}

void GrokBufferProtocol::Server::respond_error(int dest, uint8_t error_code, const char *error_description) {
    auto len = strlen(error_description);
    char *buffer = new char[sizeof(uint8_t)+len];
    buffer[0] = error_code;

    memcpy(buffer+1u, error_description, len);
    this->respond(dest, this->error_type, buffer, sizeof(uint8_t)+len);

    delete[] buffer;
}

//// Client

void GrokBufferProtocol::Client::fallback_to_type() {
    loop->recv(this->sockfd, sizeof(type_t), NONSTATIC_CALLBACK(this->type_getter));
}

int GrokBufferProtocol::Client::on_connect() {
    this->fallback_to_type();
    this->is_connected = true;

    return this->sockfd;
}

void GrokBufferProtocol::Client::post_connect(int fd) {
    IObserver::post_connect(fd);

    if (this->motd_callback != nullptr) {
        this->motd_callback(fd);
    }
}

void GrokBufferProtocol::Client::on_disconnect(int fd) {
    this->disconnect_callback(fd);
}

//// Getters

void GrokBufferProtocol::Client::type_getter(Cameleo::Future *future) {
    BufferReader reader(future->recv_buffer, this->endianess);
    auto type = reader.read<type_t>();

    loop->recv(future->fd, sizeof(length_t),
            NONSTATIC_CALLBACK1(this->length_getter, type));
}

void GrokBufferProtocol::Client::length_getter(Cameleo::Future *future,
        GrokBufferProtocol::Client::type_t type) {
    BufferReader reader(future->recv_buffer, this->endianess);
    auto length = reader.read<length_t>();

    if (length == 0u) {
        this->packet_callback(reader, future->fd,
                type, 0);

        this->fallback_to_type();

        return;
    }

    loop->recv(future->fd, length, NONSTATIC_CALLBACK2(this->buffer_getter, type, length));
}

void GrokBufferProtocol::Client::buffer_getter(Cameleo::Future *future, GrokBufferProtocol::Client::type_t type,
                                               GrokBufferProtocol::Client::length_t length) {
    BufferReader reader(future->recv_buffer, this->endianess);

    this->packet_callback(reader, future->fd, type, length);
    this->fallback_to_type();
}

void GrokBufferProtocol::Client::respond(GrokBufferProtocol::Client::type_t type, char *buffer,
                                         GrokBufferProtocol::Client::length_t length) {
    BufferWriter<sizeof(type_t)+sizeof(length_t)> writer;
    writer.write<type_t>(type);
    writer.write<length_t>(length);

    this->loop->send(this->sockfd, writer.buffer, writer.offset);

    if (buffer != nullptr) {
        this->loop->send(this->sockfd, buffer, length);
    }
}

////
