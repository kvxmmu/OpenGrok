//
// Created by kvxmmu on 3/25/21.
//

#include "client_proxy.hpp"

void ProxyObserver::capturer(Future *future) {
    char buffer[RECV_CHUNK_SIZE];
    int bytes_read = read_bytes(future->fd, buffer, RECV_CHUNK_SIZE);

    this->proxier->forward(this->client_id, buffer, bytes_read);
}

void ProxyObserver::on_disconnect(int fd) {

}

void ProxyObserver::force_disconnect() {
    this->loop->remove_observer(this->sockfd);

    operator delete(this);
}

void ProxyObserver::forward(char *buffer, uint32_t length) {
    this->loop->send(this->sockfd, buffer, length);
}
