//
// Created by kvxmmu on 3/27/21.
//

#include "server.hpp"

#include <iostream>

void OpenGrok::MainServer::on_packet(BufferReader &reader, int fd, OpenGrok::MainServer::type_t type,
                                     OpenGrok::MainServer::length_t length) {
    switch (type) {
        case CREATE_SERVER: {
            auto listen_port = OPENGROK_PROXY_PORT;
            auto proxy_server = new OpenGrok::ProxyServer(listen_port, this,
                    fd);

            loop.add_observer(proxy_server);
            this->servers[fd] = proxy_server;

            this->respond_port(fd, proxy_server->get_listening_port());

            break;
        }

        case SEND_PACKET: {
            auto client_id = reader.read<client_id_t>();
            auto actual_length = length - sizeof(client_id_t);
            auto proxy_server = this->servers.at(fd);

            proxy_server->forward(client_id, reader.buffer+reader.offset,
                    actual_length);

            break;
        }

        case CLIENT_DISCONNECTED: {
            auto client_id = reader.read<client_id_t>();
            auto proxy_server = this->servers.at(fd);

            proxy_server->disconnect(client_id);

            break;
        }

        case GET_SERVER_NAME: {
            protocol.respond(fd, GET_SERVER_NAME, const_cast<char *>(this->app_name.data()),
                    this->app_name.size());

            break;
        }

        default: {
            protocol.respond_error(fd, NO_SUCH_COMMAND, NO_SUCH_COMMAND_E);

            break;
        }
    }
}

void OpenGrok::MainServer::on_disconnect(int fd) {
    if (this->servers.find(fd) != this->servers.end()) {
        std::cout << "[OpenGrok::MainServer] ProxyServer destroyed#" << fd << std::endl;

        auto server = this->servers[fd];

        server->shutdown();
        loop.remove_observer(dynamic_cast<ProxyServer *>(server));
    }
}

//// Responders

void OpenGrok::MainServer::respond_port(int dest, uint16_t port) {
    BufferWriter<sizeof(uint16_t)> writer;
    writer.write<uint16_t>(port);

    protocol.respond(dest, CREATE_SERVER,
            writer.buffer, writer.offset);
}

//// IMainServer

void OpenGrok::MainServer::connect(int dest, client_id_t client_id) {
    BufferWriter<sizeof(client_id_t)> writer;
    writer.write<client_id_t>(client_id);

    protocol.respond(dest, CLIENT_CONNECTED,
            writer.buffer, writer.offset);
}

void OpenGrok::MainServer::forward(int dest, client_id_t client_id, char *buffer, size_t length) {
    auto total_buf_size = sizeof(client_id_t)+length;
    auto sendbuffer = local_allocator.allocate(total_buf_size);

    int_to_bytes<client_id_t>(client_id, sendbuffer);
    memcpy(sendbuffer+sizeof(client_id_t), buffer, length);

    protocol.respond(dest, SEND_PACKET, sendbuffer, total_buf_size);
    local_allocator.deallocate(sendbuffer, total_buf_size);
}

void OpenGrok::MainServer::disconnect(int dest, client_id_t client_id) {
    BufferWriter<sizeof(client_id_t)> writer;
    writer.write<client_id_t>(client_id);

    protocol.respond(dest, CLIENT_DISCONNECTED,
            writer.buffer, writer.offset);
}

////
