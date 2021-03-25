//
// Created by kvxmmu on 3/25/21.
//

#include "opengrok.hpp"

void OpenGrok::MainServer::packet_getter(Future *future, uint64_t client_id,
        uint32_t length) {
    auto server = this->get_client_server(client_id);

    server->forward(client_id, future->buffer, length);
    this->fallback_to_type(future->fd);
}

void OpenGrok::MainServer::disconnect_target_getter(Future *future) {
    BufferReader reader(future->buffer);

    auto client_id = reader.read<uint64_t>();
    auto server = this->get_client_server(client_id);

    server->disconnect(client_id);
    this->unlink_client(client_id);
}

void OpenGrok::MainServer::type_getter(Future *future) {
    BufferReader reader(future->buffer);
    auto type = reader.read<uint8_t>();

    switch (type) {
        case CREATE_SERVER: {
            uint16_t generated_port = 2281;
            this->write_port(future->fd, generated_port);

            auto srv = new OpenGrok::ProxyServer(future->fd, generated_port,
                    this);

            this->loop->add_observer(srv);
            this->fallback_to_type(future->fd);

            break;
        }

        case SEND_PACKET: {
            this->loop->recv(future->fd, sizeof(uint64_t)+sizeof(uint32_t),
                    [this](Future *stage_future) {
                BufferReader reader(stage_future->buffer);

                auto client_id = reader.read<uint64_t>();
                auto length = reader.read<uint32_t>();

                this->loop->recv(stage_future->fd, length, NONSTATIC_CALLBACK2(this->packet_getter, client_id, length));
            });

            break;
        }

        case CLIENT_DISCONNECT: {
            this->loop->recv(future->fd, sizeof(uint64_t), NONSTATIC_CALLBACK(this->disconnect_target_getter));

            break;
        }

        default: {
            this->write_error(future->fd, NO_SUCH_COMMAND, NO_SUCH_COMMAND_E);
            this->fallback_to_type(future->fd);

            break;
        }
    }
}

int OpenGrok::MainServer::on_connect() {
    auto client = GrokLoop::accept4(this->sockfd);

    this->logger << "Connected client#" << client.str().data() << GLog::endl;
    this->fallback_to_type(client.sockfd);

    return client.sockfd;
}

void OpenGrok::MainServer::on_disconnect(int fd) {

}

//// writers

void OpenGrok::MainServer::write_port(int fd, uint16_t lport) {
    BufferWriter<sizeof(uint8_t)+sizeof(uint16_t)> writer;

    writer.write<uint8_t>(SERVER_CREATED);
    writer.write<uint16_t>(lport);

    this->loop->send(fd, writer.buffer, writer.offset);
}

void OpenGrok::MainServer::write_error(int fd, uint8_t error_code, const char *error_description) {
    DynamicBufferWriter writer;

    writer.write<uint8_t>(ERROR);
    writer.write<uint8_t>(error_code);
    writer.write_string(error_description);

    this->loop->send(fd, writer.buffer, writer.end);
}

void OpenGrok::MainServer::connect(int dest, GIDPool::value_type client_id) {
    BufferWriter<sizeof(uint8_t)+sizeof(GIDPool::value_type)> writer;

    writer.write<uint8_t>(CLIENT_CONNECTED);
    writer.write<GIDPool::value_type>(client_id);

    this->loop->send(dest, writer.buffer, writer.offset);
}

void OpenGrok::MainServer::forward(int dest, GIDPool::value_type client_id, char *buffer, uint32_t length) {
    DynamicBufferWriter writer;

    writer.write<uint8_t>(CLIENT_SENT_DATA);
    writer.write<uint64_t>(client_id);
    writer.write<uint32_t>(length);
    writer.write(buffer, length);

    this->loop->send(dest, writer.buffer, writer.end);
}

void OpenGrok::MainServer::disconnect(int dest, GIDPool::value_type client_id) {
    BufferWriter<sizeof(uint8_t)+sizeof(GIDPool::value_type)> writer;

    writer.write<uint8_t>(CLIENT_DISCONNECTED);
    writer.write<GIDPool::value_type>(client_id);

    this->loop->send(dest, writer.buffer, writer.offset);
}
