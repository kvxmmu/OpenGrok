//
// Created by kvxmmu on 5/3/20.
//

#include "servers.hpp"


// helpers
void on_progress(QueueItem *item) {
    Progress &progress = *reinterpret_cast<Progress *>(item->extended);

    if (item->sent >= item->length) {
        progress.pool->set_used(progress.fd, false);

        delete &progress;
    }
}

void remove_progress(int fd, void *arg) {
    auto server = reinterpret_cast<ProxyServer *>(arg);

    server->remove_from_all(fd);
}

// ProxyServer

ProxyServer::ProxyServer(unsigned short _port, in_addr_t _in_address) : AbstractProtocol(_port, _in_address) {
    this->bind();
}

void ProxyServer::on_connect() {
    int client_fd = accept(this->sockfd, nullptr, nullptr);
    this->loop->add_client(client_fd);

    auto proxier = new ProxierServer(2281);
    proxier->port = get_listen_port(proxier->sockfd);
    proxier->main_server = this;
    proxier->initiator = client_fd;

    std::cout << NAME << SPACE << "Started server at port " << proxier->port << " for Client#" << client_fd << std::endl;

    this->loop->new_server(proxier);
    char sendbuff[HEADER_SIZE+sizeof(unsigned short)];
    copy_port(sendbuff, proxier->port);

    this->loop->push_packet(client_fd, sendbuff, HEADER_SIZE+sizeof(unsigned short));
}

void ProxyServer::on_data_received(int fd) {

}

void ProxyServer::on_disconnect(int fd) {
    int srvfd = this->find_by_initiator(fd);

    if (this->usage.is_used(srvfd))
        return;

    std::cout << NAME << SPACE << "Client#" << this->sockfd << " closed connection" << std::endl;

    this->remove_from_all(fd);
    this->remove_from_all(srvfd);

    this->loop->close_all_connections_from_server(srvfd, remove_progress, this);
    this->remove_server(srvfd);
    this->usage.clear_used(srvfd);
    int res = close(srvfd);
    close(fd);
}

void ProxyServer::init(EventLoop *_loop) {
    this->loop = _loop;

    this->loop->sockfd = this->sockfd;
}

void ProxyServer::remove_server(int server_fd) {
    for (auto &protocol_pair : this->loop->protocols) {
        if (protocol_pair.first == server_fd) {
            auto srv = reinterpret_cast<ProxierServer *>(protocol_pair.second);
            srv->dispose();

            std::cout << NAME << SPACE << "Server Pointer#" << srv << " freed" << std::endl;

            delete srv;
            this->loop->protocols.erase(protocol_pair.first);
        }
    }
}

int ProxyServer::find_by_initiator(int initiator) {
    for (auto &protocol_pair : this->loop->protocols) {
        auto srv = reinterpret_cast<ProxierServer *>(protocol_pair.second);

        if (srv->initiator == initiator)
            return srv->sockfd;
    }
    return -1;
}

void ProxyServer::send_disconnect(int who, int initiator, int srvfd) {
    if (this->usage.is_used(srvfd))
        return;
    char dis[HEADER_SIZE+sizeof(int)];

    copy_disconnect(dis, who);
    this->loop->push_packet(initiator, dis, HEADER_SIZE+sizeof(int),
            on_progress, this->create_progress(initiator));
}

Progress *ProxyServer::create_progress(int fd) {
    this->usage.set_used(fd, true);
    auto progress = new Progress;
    progress->fd = fd;
    progress->pool = &this->usage;

    return progress;
}

void ProxyServer::remove_from_all(int fd) {
    this->loop->main_selector.remove(fd, JUST_REMOVE);

    if (this->loop->packets_queue.queues.find(fd) != this->loop->packets_queue.queues.end()) {
        std::deque<QueueItem *> &qitems = this->loop->packets_queue.queues.at(fd);
        for (auto &item : qitems) {
            item->sent = item->length;

            if (item->on_progress != nullptr)
                item->on_progress(item);
        }
        this->loop->packets_queue.queues.erase(fd);
    }
}

// Proxier

void ProxierServer::init(EventLoop *_loop) {
    this->loop = _loop;

    this->loop->sockfd = this->sockfd;
}

ProxierServer::ProxierServer(unsigned short _port, in_addr_t _in_address) : AbstractProtocol(_port, _in_address) {
    this->bind();
}

void ProxierServer::on_connect() {
    if (this->main_server->usage.is_used(this->sockfd))
        return;
    int client_fd = accept(this->sockfd, nullptr, nullptr);
    this->main_server->usage.set_used(this->sockfd, true);
    this->loop->add_client(client_fd);

    std::cout << NAME << SPACE << "Client#" << client_fd << " connected to the Server#" << this->sockfd << std::endl;

    char buff[HEADER_SIZE+sizeof(int)];
    copy_connect(buff, client_fd);
    this->loop->push_packet(this->initiator, buff, HEADER_SIZE+sizeof(int), on_progress,
            this->main_server->create_progress(this->sockfd));
}

void ProxierServer::on_data_received(int fd) {
    if (this->main_server->usage.is_used(this->sockfd))
        return;

    const size_t header_size = HEADER_SIZE+sizeof(int)+sizeof(int);

    char buffer[BUFF_SIZE-header_size];

    int bytes = read_bytes(fd, buffer, BUFF_SIZE-header_size);

    char sendbuffer[BUFF_SIZE];
    copy_packet(sendbuffer, fd, bytes, buffer);
    this->loop->push_packet(this->initiator, sendbuffer, HEADER_SIZE+sizeof(int)+sizeof(int)+bytes, on_progress, this->main_server->create_progress(this->sockfd));

}

void ProxierServer::on_disconnect(int fd) {
    this->main_server->remove_from_all(fd);
    this->loop->proto_clients.erase(fd);
    std::cout << NAME << SPACE << "Client#" << fd << " closed connection from Server#" << this->sockfd << std::endl;
    this->main_server->send_disconnect(fd, this->initiator, this->sockfd);
    close(fd);
}
