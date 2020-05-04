//
// Created by kvxmmu on 5/4/20.
//

#include "clients.hpp"

// helpers

int connect_and_get_fd(unsigned short port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in caddr = create_addr(port, inet_addr(RELAY_HOST));
    int res = connect(sockfd, reinterpret_cast<sockaddr *>(&caddr), sizeof(caddr));

    if (res < 0)
        perror("connect()");

    return sockfd;
}

void on_fcking_progress(QueueItem *item) {
    auto client = reinterpret_cast<ProxyClient *>(item->extended);

    if (client->sent >= item->length) {
        client->state = GET_TYPE;
        client->length = 0;
        client->sent = 0;
        client->to = 0;
        return;
    }
}

// ProxyClient

ProxyClient::ProxyClient(unsigned short _port, in_addr_t _addr) : AbstractProtocol(_port, _addr) {

}

void ProxyClient::init(EventLoop *_loop) {
    this->loop = _loop;

    this->loop->sockfd = this->sockfd;
}

void ProxyClient::on_connect() {}

void ProxyClient::on_data_received(int fd) {
    if (this->state == GET_MESSAGE) {
        char buffer[BUFF_SIZE];

        int bytes = -1;

        int remaining = this->length - this->sent;

        if (remaining >= BUFF_SIZE) {
            bytes = read_fatal(fd, buffer, BUFF_SIZE);
        } else if (remaining <= 0) {
            this->state = GET_TYPE;
            return;
        } else {
            bytes = read_fatal(fd, buffer, remaining);
        }
        this->sent += bytes;
        int towho = this->connections.at(this->to);
        this->loop->push_packet(towho, buffer, bytes);

        if (this->sent >= this->length) {
            this->length = 0;
            this->sent = 0;
            this->to = 0;
            this->state = GET_TYPE;
            return;
        }

        return;
    }

    if (this->state == GET_TYPE) {
        uint8_t type;
        read_fatal(fd, &type, sizeof(uint8_t));

        if (type == CONNECT)
            this->state = GET_WHO_CONNECTED;
        else if (type == DISCONNECT)
            this->state = GET_WHO_DISCONNECTED;
        else if (type == WRITE)
            this->state = GET_WHO_SENT;
    } else if (this->state == GET_WHO_CONNECTED) {
        int who;
        read_fatal(fd, &who, sizeof(int));
        std::cout << NAME << SPACE << "Connected Client#" << who << std::endl;

        int newsockfd = connect_and_get_fd(this->connect_port);
        this->connections.emplace(std::make_pair(who, newsockfd));

        auto conn = new ConnectionClient(newsockfd, inet_addr(RELAY_HOST));
        conn->client = this;

        this->loop->new_server(conn);

        this->state = GET_TYPE;
    } else if (this->state == GET_WHO_DISCONNECTED) {
        int who;
        read_fatal(fd, &who, sizeof(int));
        std::cout << NAME << SPACE << "Disconnected Client#" << who << std::endl;

        int local_fd = this->connections.at(who);
        this->remove_from_all(local_fd);
        close(local_fd);
        this->connections.erase(who);
        this->loop->protocols.erase(local_fd);

        this->state = GET_TYPE;
    } else if (this->state == GET_WHO_SENT) {
        read_fatal(fd, &this->to, sizeof(int));

        this->state = GET_LENGTH;
    } else if (this->state == GET_LENGTH) {
        read_fatal(fd, &this->length, sizeof(int));

        this->state = GET_MESSAGE;
    }
}

void ProxyClient::on_disconnect(int fd) {
    std::cout << NAME << SPACE << "Fatal: Disconnected" << std::endl;
    exit(0);
}

bool ProxyClient::is_server() {
    return false;
}

void ProxyClient::remove_from_all(int fd) {
    this->loop->main_selector.remove(fd, JUST_REMOVE);

    if (this->loop->packets_queue.queues.find(fd) != this->loop->packets_queue.queues.end()) {
        std::deque<QueueItem *> &qitems = this->loop->packets_queue.queues.at(fd);

        for (auto &qitem : qitems) {
            qitem->sent = qitem->length;

            if (qitem->on_progress != nullptr)
                qitem->on_progress(qitem);
        }

        this->loop->packets_queue.queues.erase(fd);
    }
}

void ProxyClient::send_disconnect(int who) {
    int real = this->get_internal_fd(who);

    char dis[HEADER_SIZE+sizeof(int)];
    copy_disconnect(dis, real);

    this->loop->push_packet(this->sockfd, dis, HEADER_SIZE+sizeof(int));
}

int ProxyClient::get_internal_fd(int local_fd) {
    for (auto &fd_pair : this->connections) {
        if (fd_pair.second == local_fd)
            return fd_pair.first;
    }

    std::cout << NAME << SPACE << "Failed to get internal FD#" << local_fd << std::endl;

    return -1;
}

// ConnectionClient

void ConnectionClient::on_connect() {}

void ConnectionClient::on_data_received(int fd) {

    char buffer[BUFF_SIZE];

    int bytes = read_bytes(fd, buffer, BUFF_SIZE);

    char sendbuffer[HEADER_SIZE+sizeof(int)+sizeof(int)+bytes];
    copy_packet(sendbuffer, this->client->get_internal_fd(fd), bytes, buffer);

    this->loop->push_packet(this->client->sockfd, sendbuffer, HEADER_SIZE+sizeof(int)+sizeof(int)+bytes);
}

void ConnectionClient::on_disconnect(int fd) {
    std::cout << NAME << SPACE << "Client#" << fd << " closed connection" << std::endl;

    this->client->remove_from_all(fd);
    this->client->send_disconnect(fd);

    close(fd);
}

bool ConnectionClient::is_server() {
    return false;
}

ConnectionClient::ConnectionClient(int fd, in_addr_t address) : AbstractProtocol(0, address, fd) {

}

void ConnectionClient::init(EventLoop *_loop) {
    this->loop = _loop;
}
