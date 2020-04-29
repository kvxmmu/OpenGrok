#include <grok/evloop.hpp>

#include <iostream>

#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <vector>

#define BIND_ADDR INADDR_ANY

#define write(fd, buffer, bufflen) write_bytes(fd, buffer, bufflen)
#define read(fd, buffer, bufflen) read_bytes(fd, buffer, bufflen)

#define SEND_PORT 1
#define CONNECT 2
#define WRITE 3
#define DISCONNECT 4

#define NAME "[OpenGrok]"
#define SPACE ' '

#define GET 0
#define GET_FROM 1
#define GET_TO 1
#define GET_LENGTH 2
#define GET_DISCONNECTED 3
#define GET_MESSAGE 4

#define BUFF_SIZE 4096

namespace {
    int y = 1;
} // external linkage


typedef struct {
    int srvfd = 0;
    int initiator = 0;
    void *memptr = nullptr;

    uint8_t type = 0;
    union {
        int from = 0;
        int to;
    };
    int sent = 0;

    char buffer[BUFF_SIZE];

    bool locked = true;
    bool already_sent = false;
    Future *client_future = nullptr;

    time_t buffer_lock_time = 0;

    int length = 0;
} ServerData;


class Server;

// helpers
sockaddr_in create_addr(unsigned short port, in_addr_t listen_addr = INADDR_ANY);
unsigned short get_listen_port(int fd);
int write_bytes(int fd, void *buffer, size_t bufflen);
int read_bytes(int fd, void *buffer, size_t bufflen);
template <typename T> void write_struct(const T &data, int fd);
bool check_read(int bytes_read, ServerData &data, Server *server, EventLoop *loop, Future *fut);

// callbacks
void on_connect(EventLoop *loop, Future *fut);
void on_client_connect(EventLoop *loop, Future *fut);
void on_server_message(EventLoop *loop, Future *fut);
void on_client_message(EventLoop *loop, Future *fut);
void on_client_can_write(EventLoop *loop, Future *fut);

void on_server_exit(EventLoop *loop, Future *fut);

// Main classes

typedef struct {
    int server; // connection initiator
    int client; // connected client
} Connection;

typedef struct {
    int sockfd;
    int initiator;
} InternalServer;

struct SendPort {
    const static uint8_t type = SEND_PORT;
    unsigned short port;
};

struct DataHeader {
    const static uint8_t type = WRITE;
    int user = 0;
    int length = 0;
};

class Server {
public:
    EventLoop loop;
    sockaddr_in addr{};

    std::vector<Connection> connections;
    std::vector<InternalServer> servers;

    unsigned short listen_port{};

    int sockfd = 0;

    void remove_server(int srvfd);
    void remove_all_server_clients(int srvfd);
    void remove_connection(const Connection &conn);

    void run_until_complete();
    void init(unsigned short port);
};