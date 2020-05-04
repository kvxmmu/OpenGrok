//
// Created by kvxmmu on 5/4/20.
//

#ifndef OPENGROK2_CLIENTS_HPP
#define OPENGROK2_CLIENTS_HPP

#include <opengrok/loop.hpp>
#include <opengrok/def.hpp>
#include <opengrok/utils.hpp>

#include <arpa/inet.h>

#define PROXY_PORT 8091
#define PROXY_HOST "127.0.0.1"

#define RELAY_HOST "127.0.0.1"

#define GET_TYPE 0
#define GET_WHO_CONNECTED 1
#define GET_WHO_DISCONNECTED 2
#define GET_WHO_SENT 3
#define GET_LENGTH 4
#define GET_MESSAGE 5

#define read_fatal(fd, buffer, bufflen) read_bytes(fd, buffer, bufflen, true)

typedef std::unordered_map<int, int> connections_t;

int connect_and_get_fd(unsigned short port);

void on_fcking_progress(QueueItem *item);

class ProxyClient : public AbstractProtocol {
public:
    uint32_t state = GET_TYPE;

    UsagePool usage;
    connections_t connections;

    unsigned short connect_port;

    int to{};
    int length{};

    int sent = 0;

    ProxyClient(unsigned short _port, in_addr_t _addr);

    void init(EventLoop *_loop) override;

    void on_connect() override;
    void on_data_received(int fd) override;
    void on_disconnect(int fd) override;

    void send_disconnect(int who);

    int get_internal_fd(int local_fd);

    bool is_server() override;

    void remove_from_all(int fd);
};

class ConnectionClient : public AbstractProtocol {
public:
    ProxyClient *client = nullptr;

    ConnectionClient(int fd, in_addr_t address);

    void init(EventLoop *_loop) override;

    void on_connect() override;
    void on_data_received(int fd) override;
    void on_disconnect(int fd) override;

    bool is_server() override;
};


#endif //OPENGROK2_CLIENTS_HPP
