//
// Created by kvxmmu on 3/25/21.
//

#include "client_observer.hpp"

int ClientObserver::on_connect() {
    this->logger << "Connected to the opengrok main server" << GLog::endl;
    this->is_connected = true;

    this->fallback_to_type();

    return this->sockfd;
}

void ClientObserver::on_disconnect(int fd) {
    this->logger << "Suddenly OpenGrok main server closed connection" << GLog::endl;
}

void ClientObserver::packet_getter(Future *future, uint64_t client_id, uint32_t length) {


    this->fallback_to_type();
}

void ClientObserver::type_getter(Future *future) {
    BufferReader reader(future->buffer);
    auto type = reader.read<uint8_t>();

    switch (type) {
        case SERVER_CREATED: {
            this->loop->recv(this->sockfd, sizeof(uint16_t), [this](Future *fut) {
                BufferReader breader(fut->buffer);

                this->logger << "Listening at port " << breader.read<uint16_t>() << GLog::endl;
                this->fallback_to_type();
            });

            break;
        }

        case CLIENT_CONNECTED: {
            this->loop->recv(this->sockfd, sizeof(uint64_t), [this](Future *fut) {
                BufferReader breader(fut->buffer);
                auto client_id = breader.read<uint64_t>();

                this->logger << "Connected ID#" << client_id << GLog::endl;

                auto proxy = new ProxyObserver(this->relay_ip, this->relay_port,
                        client_id, this);

                this->loop->add_observer(proxy);

                this->fallback_to_type();
            });

            break;
        }

        case CLIENT_SENT_DATA: {
            this->loop->recv(this->sockfd, sizeof(uint64_t)+sizeof(uint32_t), [this](Future *fut) {
                BufferReader breader(fut->buffer);
                auto client_id = breader.read<uint64_t>();
                auto length = breader.read<uint32_t>();

                this->logger << "Sending " << static_cast<uint64_t>(length) << " bytes" << GLog::endl;

                this->loop->recv(this->sockfd, length, NONSTATIC_CALLBACK2(this->packet_getter, client_id, length));
            });

            break;
        }

        default: {
            throw std::runtime_error("Unknown type");

            break;
        }
    }
}

void ClientObserver::forward(uint64_t client_id, char *buffer, uint32_t length) {
    DynamicBufferWriter writer;

    writer.write<uint8_t>(SEND_PACKET);
    writer.write<uint64_t>(client_id);
    writer.write<uint32_t>(length);
    writer.write(buffer, length);

    this->loop->send(this->sockfd, writer.buffer, writer.end);
}

void ClientObserver::disconnect(uint64_t client_id) {
    BufferWriter<sizeof(uint8_t)+sizeof(uint64_t)> writer;

    writer.write<uint8_t>(CLIENT_DISCONNECT);
    writer.write<uint64_t>(client_id);

    this->loop->send(this->sockfd, writer.buffer, writer.offset);
}
