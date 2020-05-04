//
// Created by kvxmmu on 5/3/20.
//

#ifndef OPENGROK2_SERVERS_HPP
#define OPENGROK2_SERVERS_HPP

#include <opengrok/loop.hpp>
#include <vector>

#include <opengrok/utils.hpp>

#define GET_TYPE 0

struct InternalServer {
    int fd;
    int initiator;
};

struct Progress {
    UsagePool *pool = nullptr;

    int fd = 0;
};

void remove_progress(int fd, void *arg);

class ProxyServer;

void on_progress(QueueItem *item);

class ProxierServer : public AbstractProtocol {
public:
    ProxyServer *main_server = nullptr;
    int initiator = 0;

    explicit ProxierServer(unsigned short _port, in_addr_t _in_address = INADDR_ANY);

    void init(EventLoop *_loop) override;

    void on_connect() override;

    void on_data_received(int fd) override;

    void on_disconnect(int fd) override;
};

class ProxyServer : public AbstractProtocol {
public:
    std::vector<InternalServer> servers;

    int state = GET_TYPE;
    UsagePool usage;


    explicit ProxyServer(unsigned short _port, in_addr_t _in_address = INADDR_ANY);

    void init(EventLoop *_loop) override;

    void on_connect() override;

    void on_data_received(int fd) override;

    void on_disconnect(int fd) override;

    void remove_server(int server_fd);
    int find_by_initiator(int initiator);

    void send_disconnect(int who, int initiator, int srvfd);
    void send_connect(int);

    void remove_from_all(int fd);

    Progress *create_progress(int fd);
};


#endif //OPENGROK2_SERVERS_HPP
