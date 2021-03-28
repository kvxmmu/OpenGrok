//
// Created by kvxmmu on 3/28/21.
//

#include "proxy_client.hpp"

#include <iostream>

void OpenGrok::ProxyClient::capturer(Future *future) {
    char buff[4096];

    auto bytes_read = Net::read_bytes(future->fd, buff, 4096);

    interface->forward(this->client_id, buff, bytes_read);
}

int OpenGrok::ProxyClient::on_connect() {
    this->is_connected = true;

    std::cout << "Connected to " << inet_ntoa(this->caddr.sin_addr) << ':' << ntohs(this->caddr.sin_port) << std::endl;
    loop->capture(this->sockfd, NONSTATIC_CALLBACK(this->capturer));

    return this->sockfd;
}

void OpenGrok::ProxyClient::on_disconnect(int fd) {
    std::cout << "Disconnected from " << inet_ntoa(this->caddr.sin_addr) << ':' << ntohs(this->caddr.sin_port) << std::endl;

    interface->disconnect(this->client_id);
}

void OpenGrok::ProxyClient::forward(char *buffer, size_t count) {
    loop->send(this->sockfd, buffer, count);
}
