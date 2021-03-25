//
// Created by kvxmmu on 3/25/21.
//

#include "opengrok_proxy.hpp"

void OpenGrok::ProxyServer::capturer(Future *future) {
    char buffer[RECV_CHUNK_SIZE];

    int bytes_read = read_bytes(future->fd, buffer, RECV_CHUNK_SIZE);

    this->server->forward(this->proxier_fd, this->server->get_client_id(future->fd),
            buffer, bytes_read);
}

int OpenGrok::ProxyServer::on_connect() {
    auto client = GrokLoop::accept4(this->sockfd);
    auto client_id = this->server->link_client(client.sockfd, this);

    this->logger << "Connected " << client.str().data() << " and acquired ID#" << client_id << GLog::endl;

    this->server->connect(this->proxier_fd, client_id);
    this->loop->capture(client.sockfd, NONSTATIC_CALLBACK(this->capturer));

    return client.sockfd;
}

void OpenGrok::ProxyServer::on_disconnect(int fd) {
    auto client_id = this->server->get_client_id(fd);
    this->server->unlink_client(client_id);

    this->logger << "Disconnected " << IPv4(fd).str().data() << " and released ID#" << client_id << GLog::endl;

    this->server->disconnect(this->proxier_fd, client_id);
}

void OpenGrok::ProxyServer::forward(GIDPool::value_type client_id, char *buffer, uint32_t length) {
    auto client_fd = this->server->get_client_fd(client_id);

    this->logger << "Sent " << static_cast<uint64_t>(length) << " bytes to ID#" << client_id << GLog::endl;

    this->loop->send(client_fd, buffer, length);
}

void OpenGrok::ProxyServer::disconnect(GIDPool::value_type client_id) {
    auto client_fd = this->server->get_client_fd(client_id);

    this->loop->force_disconnect(client_fd);
}
