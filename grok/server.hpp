//
// Created by kvxmmu on 5/1/20.
//

#ifndef OPENGROK_SERVER_HPP
#define OPENGROK_SERVER_HPP

#include <grok/loop.hpp>
#include <grok/bin.hpp>
#include <grok/utils.hpp>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <unordered_set>

#define GET_TYPE 0
#define GET_WHO_DISCONNECTED 1
#define GET_WHO_SENT 2
#define GET_LENGTH 3
#define GET_MESSAGE 4


#define RESET {fut->state = GET_TYPE;\
data.sent = 0;\
data.length = 0;\
data.sent_block = false;\
return;}

#define HOST_ADDR INADDR_ANY

class Server;

typedef struct {
    int server;
    int client;
} Connection;

typedef struct {
    int server_fd;
    int initiator;
} InternalServer;


bool operator==(const InternalServer &srv1, const InternalServer &srv2);

typedef struct {
    int server_fd = 0;
    int initiator = 0;

    int who = 0;
    int length = 0;
    int sent = 0;

    bool sent_block = false;
    Future *curr = nullptr;

    Server *server = nullptr;
} EballData;

namespace std {
    template<>
    struct hash<InternalServer> {
    public:
        std::size_t operator()(const InternalServer &internal) const {
            return std::hash<int>()(internal.server_fd);
        }
    };
}

class Server {
public:
    EventLoop loop;
    unsigned short port;

    std::unordered_set<InternalServer> servers;
    std::vector<Connection> connections;

    // socket data
    int sockfd{};
    sockaddr_in addr{};

    // methods
    explicit Server(unsigned short _port);

    void remove_connection(const Connection &conn);
    void remove_server(int fd);
    void remove_all_connections(int srvfd);

    void poll();
};

// helpers
bool check_read(int bytes, Server *srv, Future *ft, int esrv, EballData &data);

// callbacks
void on_connect(EventLoop *loop, Future *fut);
void on_client_connect(EventLoop *loop, Future *fut);
void on_client_message(EventLoop *loop, Future *fut);
void on_server_message(EventLoop *loop, Future *fut);
void on_progress(QueueItem *item);


#endif //OPENGROK_SERVER_HPP
