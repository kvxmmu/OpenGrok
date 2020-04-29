#include <grok/server.hpp>


sockaddr_in create_addr(unsigned short port, in_addr_t listen_addr) {
    sockaddr_in addr{};
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = listen_addr;
    addr.sin_family = AF_INET;
    return addr;
}

unsigned short get_listen_port(int fd) {
    sockaddr_in addr{};
    socklen_t len = sizeof(addr);
    getsockname(fd, reinterpret_cast<sockaddr *>(&addr), &len);
    return ntohs(addr.sin_port);
}

int write_bytes(int fd, void *buffer, size_t bufflen) {
    int res = send(fd, buffer, bufflen, 0);
    if (res == -1)
        perror("write() error");
    return res;
}

int read_bytes(int fd, void *buffer, size_t bufflen) {
    int res = recv(fd, buffer, bufflen, 0);
    if (res == -1)
        perror("recv() error");
    return res;
}

template <typename T>
void write_struct(const T &data, int fd) {
    T copy = data;
    write(fd, &copy, sizeof(T));
}

bool check_read(int bytes_read, ServerData &data, Server *server, EventLoop *loop, Future *fut) {
    if (bytes_read <= 0) {
        if (data.client_future != nullptr)
            data.client_future->finish(loop);
        std::cout << NAME << SPACE << "Server#" << data.srvfd << " disconnected" << std::endl;
        server->remove_all_server_clients(data.initiator);
        std::cout << NAME << SPACE << "Server#" << data.srvfd << " cleaned up" << std::endl;
        close(data.initiator);
        close(data.srvfd);
        fut->finish(loop);
        return false;
    }
    return true;
}

void on_connect(EventLoop *loop, Future *fut) {
    auto server = reinterpret_cast<Server *>(fut->memptr);
    int client_fd = accept(fut->fd, nullptr, nullptr);

    int newsockfd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(newsockfd, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int));
    sockaddr_in caddr = create_addr(0, BIND_ADDR);
    bind(newsockfd, reinterpret_cast<sockaddr *>(&caddr), sizeof(caddr));
    listen(newsockfd, EOPMAX_EVENTS);

    unsigned short bindport = get_listen_port(newsockfd);

    std::cout << NAME << SPACE << "Client#" << client_fd << " connected and created Server#" << newsockfd << std::endl;

    server->servers.push_back(InternalServer{newsockfd, client_fd});

    Future &on_client_conn = loop->add_callback(newsockfd, on_client_connect);
    on_client_conn.memptr = new ServerData{newsockfd, client_fd, server};

    Future &on_server_send = loop->add_callback(client_fd, on_server_message);
    on_server_send.memptr = on_client_conn.memptr;
    on_server_send.on_finished = on_server_exit;

    write_struct(SendPort{bindport}, client_fd);
}

void on_server_message(EventLoop *loop, Future *fut) {
    auto &data = *reinterpret_cast<ServerData *>(fut->memptr);
    auto server = reinterpret_cast<Server *>(data.memptr);

    uint8_t type;

    int bytes_read;

    if (fut->state == GET_MESSAGE) {
        if (data.sent >= data.length) {
            data.client_future->finish(loop);
            fut->state = GET;
            return;
        }
        if (data.locked)
            return;
        char buffer[BUFF_SIZE];
        bytes_read = read(data.initiator, buffer, BUFF_SIZE);
        if (!check_read(bytes_read, data, server, loop, fut))
            return;
        int written = write(data.to, buffer, bytes_read);
        data.sent += written;
    } if (fut->state == GET) {
        bytes_read = read(data.initiator, &type, sizeof(uint8_t));

        if (!check_read(bytes_read, data, server, loop, fut))
            return;

        data.type = type;

        if (type == DISCONNECT) {
            fut->state = GET_DISCONNECTED;
        } else if (type == WRITE) {
            fut->state = GET_TO;
        } else {
            std::cerr << NAME << SPACE << "Server#" << data.srvfd << " closed, reason: unknown event type" << std::endl;
            close(data.initiator);
            close(data.srvfd);
            server->remove_all_server_clients(data.initiator);
            fut->finish(loop);
            return;
        }
    } else if (fut->state == GET_DISCONNECTED) {
        int who;
        bytes_read = read(data.initiator, &who, sizeof(int));
        if (!check_read(bytes_read, data, server, loop, fut))
            return;
        close(who); // TODO: add descriptor check
        Connection tconn{};
        tconn.server = data.initiator;
        tconn.client = who;
        server->remove_connection(tconn);
        fut->state = GET;
    } else if (fut->state == GET_TO) {
        int to;
        bytes_read = read(data.initiator, &to, sizeof(int));
        if (!check_read(data.initiator, data, server, loop, fut))
            return;
        data.to = to;
        fut->state = GET_LENGTH;
    } else if (fut->state == GET_LENGTH) {
        int length;
        bytes_read = read(data.initiator, &length, sizeof(int));
        if (!check_read(data.initiator, data, server, loop, fut))
            return;
        data.length = length;
        fut->state = GET_MESSAGE;
        Future &ewritefut = loop->add_callback(data.to, on_client_can_write);
        data.client_future = &ewritefut;
        ewritefut.modify_events(EPOLLOUT, loop);
        ewritefut.memptr = &data;
    }


}

void on_client_connect(EventLoop *loop, Future *fut) {

}

void on_client_message(EventLoop *loop, Future *fut) {

}


void on_client_can_write(EventLoop *loop, Future *fut) {
    auto &data = *reinterpret_cast<ServerData *>(fut->memptr);
    data.locked = false;
}

void on_server_exit(EventLoop *loop, Future *fut) {
    auto data = reinterpret_cast<ServerData *>(fut->memptr);
    std::cout << NAME << SPACE << "Server Pointer#" << data << " freed" << std::endl;
    delete data;
}

void Server::init(unsigned short port) {
    this->listen_port = port;
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int));
    this->addr = create_addr(port);

    bind(this->sockfd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
    listen(this->sockfd, EOPMAX_EVENTS);

    Future &fut = this->loop.add_callback(this->sockfd, on_connect);
    fut.memptr = this;
}

void Server::run_until_complete() {
    this->loop.poll();
}

void Server::remove_server(int srvfd) {
    size_t index = std::string::npos;

    for (size_t pos = 0; pos < this->servers.size(); pos++) {
        InternalServer &srv = this->servers[pos];
        if (srv.sockfd == srvfd) {
            index = pos;
            break;
        }
    }

    if (index != std::string::npos) {
        this->servers.erase(this->servers.begin()+index);
    } else {
        std::cerr << NAME << SPACE << " Unknown Server#" << srvfd << std::endl;
    }
}

void Server::remove_connection(const Connection &conn) {
    size_t index = std::string::npos;

    for (size_t pos = 0; pos < this->connections.size(); pos++) {
        Connection &econn = this->connections[pos];
        if (econn.client == conn.client && econn.server == conn.server) {
            index = pos;
            break;
        }
    }

    if (index != std::string::npos) {
        this->connections.erase(this->connections.begin()+index);
    } else {
        std::cerr << NAME << SPACE << " Unknown Connection{" << conn.server << ", " << conn.client << "}" << std::endl;
    }
}

void Server::remove_all_server_clients(int srvfd) {
    for (size_t pos = 0; pos < this->connections.size(); pos++) {
        Connection &conn = this->connections[pos];
        if (conn.server == srvfd) {
            close(conn.client);
            this->connections.erase(this->connections.begin()+pos);
            pos--;
        }
    }
}
