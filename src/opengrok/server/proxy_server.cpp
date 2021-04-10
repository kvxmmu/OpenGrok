//
// Created by kvxmmu on 3/28/21.
//

#include "proxy_server.hpp"

#include <iostream>

int OpenGrok::ProxyServer::on_connect() {
    auto client = Cameleo::EventLoop::accept4(this->sockfd);
    auto client_id = interface->link_client(client.sockfd);

    interface->connect(this->holder, client_id);
    loop->capture(client.sockfd, NONSTATIC_CALLBACK(this->capturer));

    std::cout << "[OpenGrok::ProxyServer] Connected client with fd " << client.sockfd << std::endl;

    return client.sockfd;
}

void OpenGrok::ProxyServer::capturer(Cameleo::Future *future) {
    auto client_id = interface->get_id_by_fd(future->fd);
    char buffer[OpenGrok::chunk_size];

    int bytes_read = Net::read_bytes(future->fd, buffer, OpenGrok::chunk_size);

    interface->forward(this->holder, client_id,
            buffer, bytes_read);
}

void OpenGrok::ProxyServer::on_disconnect(int fd) {
    interface->disconnect(this->holder, interface->get_id_by_fd(fd));

    std::cout << "[OpenGrok::ProxyServer] Disconnected client with fd " << fd << std::endl;
}

////

void OpenGrok::ProxyServer::forward(client_id_t client_id, char *buffer, size_t length) {
    auto fd = interface->get_fd_by_id(client_id);

    if (!this->__has_client(fd)) {
        return;
    }

    loop->send(fd, buffer, length);
}

void OpenGrok::ProxyServer::disconnect(client_id_t client_id) {
    auto fd = interface->get_fd_by_id(client_id);
    interface->unlink(client_id, fd);
    loop->force_disconnect(fd);

    this->__clients.erase(fd);

    std::cout << "[OpenGrok::ProxyServer] Disconnected client with id " << client_id << std::endl;
}

void OpenGrok::ProxyServer::shutdown() {
    for (auto fd : this->__clients) {
        auto client_id = interface->get_id_by_fd(fd);

        interface->unlink(client_id, fd);
    }
}
