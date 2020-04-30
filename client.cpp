#include <grok/evloop.hpp>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <grok/binary.hpp>

#define BUFF_SIZE 4096
#define write(fd, buffer, bufflen) write_bytes(fd, buffer, bufflen)
#define read(fd, buffer, bufflen) read_bytes(fd, buffer, bufflen)
#define read_fatal(fd, buffer, bufflen) read_bytes(fd, buffer, bufflen, true)


#define GET_TYPE 0
#define GET_WHO_CONNECTED 1
#define GET_WHO_DISCONNECTED 2

#define CONNECT 2
#define WRITE 3
#define DISCONNECT 4

#define PROXY_HOST "127.0.0.1"
#define PROXY_PORT 8089

#define NAME "[OpenGrok]"
#define SPACE ' '

int data_incoming = 0;
int sent = 0;
int sockfd;
char sock_buffer[BUFF_SIZE];


uint8_t type;

int who;

unsigned short to_port;

std::unordered_map<int, int> connections;

sockaddr_in create_addr(unsigned short port, in_addr_t listen_addr) {
    sockaddr_in addr{};
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = listen_addr;
    addr.sin_family = AF_INET;
    return addr;
}

int write_bytes(int fd, void *buffer, size_t bufflen) {
    int res = send(fd, buffer, bufflen, 0);
    if (res == -1)
        perror("write() error");
    return res;
}

int read_bytes(int fd, void *buffer, size_t bufflen, bool fatal = false) {
    int res = recv(fd, buffer, bufflen, 0);
    if (res == -1)
        perror("recv() error");
    else if (res == 0 && fatal) {
        std::cout << "[OpenGrok] Fatal: Disconnected" << std::endl;
        exit(0);
    }
    return res;
}


unsigned short receive_port() {
    unsigned short port;
    uint8_t useful_huita;

    read(sockfd, &useful_huita, sizeof(uint8_t));
    read(sockfd, &port, sizeof(unsigned short));

    return port;
}


void on_proxy_message(EventLoop *loop, Future *fut) {
    if (fut->state == GET_TYPE) {
        read_fatal(fut->fd, &type, sizeof(uint8_t));

        if (type == CONNECT)
            fut->state = GET_WHO_CONNECTED;
        else if (type == DISCONNECT)
            fut->state = GET_WHO_DISCONNECTED;
    } else if (fut->state == GET_WHO_CONNECTED) {
        read_fatal(fut->fd, &who, sizeof(int));
        std::cout << NAME << SPACE << "+1 connection#" << who << std::endl;
        fut->state = GET_TYPE;
    } else if (fut->state == GET_WHO_DISCONNECTED) {
        read_fatal(fut->fd, &who, sizeof(int));
        std::cout << NAME << SPACE << "-1 connection#" << who << std::endl;
        fut->state = GET_TYPE;
    }
}


int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << argv[0] << " usage: " << argv[0] << " [port]" << std::endl;
        return 0;
    }
    to_port = atoi(argv[1]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr = create_addr(PROXY_PORT, inet_addr(PROXY_HOST));
    int status = connect(sockfd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
    if (status == -1) {
        perror("connect() error");
        return 0;
    }
    unsigned short proxier_port = receive_port();
    std::cout << "Он придумал " << proxier_port << ", а ты даже не знаешь его " << to_port << std::endl;

    EventLoop loop;
    loop.add_callback(sockfd, on_proxy_message);

    loop.poll();
    return 0;
}