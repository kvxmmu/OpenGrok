//
// Created by kvxmmu on 3/28/21.
//

#include "client.hpp"

#include <iostream>

void OpenGrok::Client::on_packet(BufferReader &reader, int fd, OpenGrok::Client::type_t type,
                                 OpenGrok::Client::length_t length) {
    switch (type) {
        case CREATE_SERVER: {
            auto lport = reader.read<uint16_t>();

            std::cout << "Listening port " << lport << std::endl;

            break;
        }

        case CLIENT_CONNECTED: {
            sockaddr_in sockaddr{};
            sockaddr.sin_port = htons(this->port);
            sockaddr.sin_family = AF_INET;
            sockaddr.sin_addr.s_addr = inet_addr(this->ip);

            auto client_id = reader.read<client_id_t>();
            auto client_observer = new OpenGrok::ProxyClient(sockaddr, client_id,
                    this);
            loop.add_observer(client_observer);

            this->clients[client_id] = client_observer;

            std::cout << "Client connected to the proxy" << std::endl;

            break;
        }

        case GET_SERVER_NAME: {
            std::cout << "Got server name: " << std::string(reader.buffer+reader.offset, length) << std::endl;

            break;
        }

        case CLIENT_DISCONNECTED: {
            auto client_id = reader.read<client_id_t>();

            loop.remove_observer(dynamic_cast<IObserver *>(this->clients.at(client_id)));
            this->clients.erase(client_id);

            std::cout << "Server disconnected client" << std::endl;

            break;
        }

        case SEND_PACKET: {
            auto client_id = reader.read<client_id_t>();
            auto client = this->clients.at(client_id);

            client->forward(reader.buffer+reader.offset,
                    length - sizeof(client_id_t));

            break;
        }

        default: {
            std::cout << "Unknown command sent" << std::endl;

            break;
        }
    }
}

void OpenGrok::Client::on_motd(int fd) {
    this->get_server_name();
    this->create_server();
}

void OpenGrok::Client::on_disconnect(int fd) {
    std::cout << "Suddenly OpenGrok mainserver disconnected" << std::endl;
    // da
}

///

void OpenGrok::Client::create_server() {
    protocol.respond(CREATE_SERVER, nullptr, 0);
}

void OpenGrok::Client::forward(client_id_t client_id, char *buffer, size_t length) {
    auto sendbuffer = new char[sizeof(client_id_t)+length];

    int_to_bytes<client_id_t>(client_id, sendbuffer);
    memcpy(sendbuffer+sizeof(client_id_t), buffer, length);

    protocol.respond(SEND_PACKET, sendbuffer, sizeof(client_id_t)+length);

    delete[] sendbuffer;
}

void OpenGrok::Client::disconnect(client_id_t client_id) {
    BufferWriter<sizeof(client_id_t)> writer;
    writer.write<client_id_t>(client_id);

    protocol.respond(CLIENT_DISCONNECTED, writer.buffer,
            writer.offset);
}

void OpenGrok::Client::get_server_name() {
    protocol.respond(GET_SERVER_NAME, nullptr, 0);
}
