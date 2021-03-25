//
// Created by kvxmmu on 3/25/21.
//

#include "opengrok.hpp"

void OpenGrok::MainServer::type_getter(Future *future) {
    BufferReader reader(future->buffer);
    auto type = reader.read<uint8_t>();

    switch (type) {
        case CREATE_SERVER: {


            break;
        }
    }
}

int OpenGrok::MainServer::on_connect() {
    auto client = GrokLoop::accept4(this->sockfd);

    this->logger << "Connected client# " << client.str().data() << GLog::endl;
    this->fallback_to_type(client.sockfd);

    return client.sockfd;
}

void OpenGrok::MainServer::on_disconnect(int fd) {

}
