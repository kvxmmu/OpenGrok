//
// Created by kvxmmu on 5/1/20.
//

#include <iostream>
#include "server.hpp"


bool operator==(const InternalServer &srv1, const InternalServer &srv2) {
    return srv1.server_fd == srv2.server_fd;
}

Server::Server(unsigned short _port) : port(_port) {
    this->addr = create_addr(_port, HOST_ADDR);
}

void Server::poll() {
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // main fd initialization
    set_reuse(this->sockfd);
    bind(this->sockfd, reinterpret_cast<sockaddr *>(&this->addr), sizeof(this->addr));
    listen(this->sockfd, MAX_EPOLL_SIZE);

    Future &connect_future = this->loop.create_future(this->sockfd, on_connect);
    connect_future.future_data = this;

    // run event loop
    this->loop.run();
}

void Server::remove_connection(const Connection &conn) {
    size_t index = std::string::npos;
    size_t pos = 0;


    for (Connection &rconn : this->connections) {
        if (rconn.client == conn.client && rconn.server == conn.server) {
            index = pos;
            break;
        }

        pos++;
    }

    if (index != std::string::npos) {
        this->connections.erase(this->connections.begin() + index);
    } else {
        std::cout << NAME << SPACE << "Unknown Connection{" << conn.server << ", " << conn.client << '}' << std::endl;
    }
}

void Server::remove_server(int fd) {
    InternalServer r{};
    r.server_fd = fd;
    this->servers.erase(r);
}

void Server::remove_all_connections(int srvfd) {
    for (size_t pos = 0; pos < this->connections.size(); pos++) {
        Connection &conn_do = this->connections.at(pos);

        if (conn_do.server == srvfd) {
            this->loop.selector.remove(conn_do.client, EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLET);
            close(conn_do.client);
            this->connections.erase(this->connections.begin()+pos);
            pos--;
        }
    }
}

void on_progress(QueueItem *item) {
    EballData &data = *reinterpret_cast<EballData *>(item->extended);

    if (data.sent >= data.length) {
        data.curr->state = GET_TYPE;
        data.length = 0;
        data.sent = 0;
        data.sent_block = false;
    } else if (item->sent >= item->length) {
        data.sent_block = false;
    }
}

bool check_read(int bytes, Server *srv, Future *ft, int esrv, EballData &data) {

    if (bytes <= 0) {
        std::cout << NAME << SPACE << "Client#" << ft->fd << " closed connection" << std::endl;
        srv->loop.remove_by_fd(ft->fd, JUST_REMOVE);
        srv->loop.remove_by_fd(data.server_fd, JUST_REMOVE);
        close(data.server_fd);

        srv->remove_all_connections(data.server_fd);

        srv->remove_server(data.server_fd);
        close(data.initiator);
        return false;
    }

    return true;
}

void on_server_message(EventLoop *loop, Future *fut) {
    EballData &data = *reinterpret_cast<EballData *>(fut->future_data);

    if (fut->state == GET_MESSAGE) {
        if (data.sent_block)
            return;

        int remaining = data.length - data.sent;

        char temporary[BUFF_SIZE];
        int rsent;

        if (remaining >= BUFF_SIZE) {
            rsent = read_bytes(fut->fd, temporary, BUFF_SIZE);
        } else {
            rsent = read_bytes(fut->fd, temporary, remaining);
        }

        if (!check_read(rsent, data.server, fut, data.initiator, data))
            RESET

        data.sent += rsent;
        data.curr = fut;

        QueueItem &item = loop->push_packet(temporary, rsent, data.who, std::string::npos, false);
        item.extended = &data;
        item.sent = 0;
        item.on_progress = on_progress;

        data.sent_block = true;
        return;
    }

    if (fut->state == GET_TYPE) {
        uint8_t type;

        int bytes = read_bytes(fut->fd, &type, sizeof(uint8_t));

        if (!check_read(bytes, data.server, fut, data.initiator, data))
            RESET

        if (type == CLOSE)
            fut->state = GET_WHO_DISCONNECTED;
        else if (type == WRITE)
            fut->state = GET_WHO_SENT;
        else
            std::cout << NAME << SPACE << "Unknown event type " << static_cast<int>(type) << std::endl;
    } else if (fut->state == GET_WHO_DISCONNECTED) {
        int who;
        int bytes = read_bytes(fut->fd, &who, sizeof(int));

        if (!check_read(bytes, data.server, fut, data.initiator, data))
            RESET

        close(who);
        fut->state = GET_TYPE;
    } else if (fut->state == GET_WHO_SENT) {
        int bytes = read_bytes(fut->fd, &data.who, sizeof(int));

        if (!check_read(bytes, data.server, fut, data.initiator, data))
            RESET

        fut->state = GET_LENGTH;
    } else if (fut->state == GET_LENGTH) {
        int bytes = read_bytes(fut->fd, &data.length, sizeof(int));

        if (!check_read(bytes, data.server, fut, data.initiator, data))
            RESET

        fut->state = GET_MESSAGE;
    }
}

void on_connect(EventLoop *loop, Future *fut) {
    auto server = reinterpret_cast<Server *>(fut->future_data);

    int client_fd = accept(fut->fd, nullptr, nullptr);

    if (client_fd <= 0) {
        perror("accept()");
    }

    int srvfd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in caddr = create_addr(2281);

    set_reuse(srvfd);
    bind(srvfd, reinterpret_cast<sockaddr *>(&caddr), sizeof(caddr));
    listen(srvfd, MAX_EPOLL_SIZE);

    unsigned short lport = get_listen_port(srvfd);

    std::cout << NAME << SPACE << "Client#" << client_fd << " connected and created Server#" << srvfd << std::endl;

    char sendbuffer[HEADER_SIZE+sizeof(unsigned short)];
    copy_port(lport, sendbuffer);
    loop->push_packet(sendbuffer, HEADER_SIZE+sizeof(unsigned short), client_fd);

    auto dat = new EballData{};
    dat->server_fd = srvfd;
    dat->initiator = client_fd;
    dat->server = server;

    Future &on_connect_fut = loop->create_future(srvfd, on_client_connect);
    on_connect_fut.future_data = dat;

    Future &on_init_ft = loop->create_future(client_fd, on_server_message);
    on_init_ft.future_data = dat;

    server->servers.insert(InternalServer{srvfd, client_fd});

}

void on_client_connect(EventLoop *loop, Future *fut) {

    EballData &data = *reinterpret_cast<EballData *>(fut->future_data);

    int client_fd = accept(fut->fd, nullptr, nullptr);

    Future &on_client_message_fut = loop->create_future(client_fd, on_client_message);
    on_client_message_fut.future_data = &data;

    std::cout << NAME << SPACE << "Client#" << client_fd << " connected to the Server#" << data.server_fd << std::endl;

    char sendbuffer[HEADER_SIZE+sizeof(int)];
    copy_connect(client_fd, sendbuffer);
    loop->push_packet(sendbuffer, HEADER_SIZE+sizeof(int), data.initiator, std::string::npos);
    data.server->connections.push_back(Connection{data.initiator, client_fd});
}

void on_client_message(EventLoop *loop, Future *fut) {
    EballData &data = *reinterpret_cast<EballData *>(fut->future_data);

    char buffer[BUFF_SIZE];

    int bytes = read_bytes(fut->fd, buffer, BUFF_SIZE);
    if (bytes <= 0) {
        std::cout << NAME << SPACE << "Client#" << fut->fd << " closed connection from Server#" << data.server_fd << std::endl;

        char buff[HEADER_SIZE+sizeof(int)];
        copy_disconnect(fut->fd, buff);
        loop->push_packet(buff, HEADER_SIZE+sizeof(int), data.initiator, std::string::npos);

        close(fut->fd);

        return;
    }

    char buff[HEADER_SIZE+sizeof(int)+sizeof(int)+bytes];
    copy_packet(fut->fd, bytes, buffer, buff);
    QueueItem &item = loop->push_packet(buff, HEADER_SIZE+sizeof(int)+sizeof(int)+bytes, data.initiator, std::string::npos);
    item.skip_fd = true;
    item.sent = 0;
}
