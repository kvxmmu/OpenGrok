//
// Created by kvxmmu on 5/1/20.
//

#include <iostream>

#include <grok/loop.hpp>
#include <arpa/inet.h>

#include <unordered_map>


#define PROXY_HOST "127.0.0.1"
#define PROXY_PORT 8089

#define GET_TYPE 0
#define GET_WHO_CONNECTED 1
#define GET_WHO_DISCONNECTED 2
#define GET_WHO_SENT 3
#define GET_LENGTH 4
#define GET_MESSAGE 5

typedef std::unordered_map<int, int> connections_t;

static bool noblock = true;

typedef struct {
    int sockfd = 0;
    connections_t &connections;
    unsigned short port = 0;

    uint8_t type = 0;

    int who = 0;
    int length = 0;

    int sent = 0;

    Future *on_ft = nullptr;

    bool used_by_client = false;
} EBall;

void on_progress(QueueItem *item) {
    EBall &ebal = *reinterpret_cast<EBall *>(item->extended);

    if (ebal.sent >= ebal.length) {
        ebal.on_ft->state = GET_TYPE;
        ebal.sent = 0;
        ebal.length = 0;
        noblock = true;
    } else if (item->sent >= item->length) {
        noblock = true;
    }
}

int find_fd(int cfd, connections_t &connections) {
    for (auto &pair : connections) {
        if (pair.second == cfd)
            return pair.first;
    }

    return -1;
}

void client_mon(EventLoop *loop, Future *fut) {
    EBall &ebal = *reinterpret_cast<EBall *>(fut->future_data);
    char buffer[BUFF_SIZE];

    int bytes = read_bytes(fut->fd, buffer, BUFF_SIZE);

    int internal_fd = find_fd(fut->fd, ebal.connections);

    if (bytes <= 0) {
        char dis[HEADER_SIZE+sizeof(int)];
        copy_disconnect(internal_fd, dis);
        loop->push_packet(dis, HEADER_SIZE+sizeof(int), ebal.sockfd, std::string::npos, true);

        std::cout << "Server#" << fut->fd << " closed connection for the Client#" << internal_fd << std::endl;
        loop->finish(fut);
        return;
    }
    char outbuffer[HEADER_SIZE+sizeof(int)+sizeof(int)+bytes];
    copy_packet(internal_fd, bytes, buffer, outbuffer);
    loop->push_packet(outbuffer, HEADER_SIZE+sizeof(int)+sizeof(int)+bytes, ebal.sockfd, std::string::npos, true);
}

void on_server_message(EventLoop *loop, Future *fut) {
    EBall &ebal = *reinterpret_cast<EBall *>(fut->future_data);

    if (ebal.used_by_client)
        return;

    if (fut->state == GET_MESSAGE) {
        if (!noblock)
            return;
        char buff[BUFF_SIZE];
        int remaining = ebal.length - ebal.sent;
        int rsent;

        if (remaining >= BUFF_SIZE) {
            rsent = read_bytes(fut->fd, buff, BUFF_SIZE, true);
        } else {
            rsent = read_bytes(fut->fd, buff, remaining, true);
        }
        ebal.sent += rsent;

        QueueItem &queue_item = loop->push_packet(buff, rsent, ebal.connections.at(ebal.who), std::string::npos);
        queue_item.skip_fd = false;
        queue_item.on_progress = on_progress;
        ebal.on_ft = fut;
        queue_item.extended = &ebal;

        noblock = false;
        return;
    }

    if (fut->state == GET_TYPE) {
        read_bytes(fut->fd, &ebal.type, sizeof(uint8_t), true);

        if (ebal.type == CONNECT)
            fut->state = GET_WHO_CONNECTED;
        else if (ebal.type == WRITE)
            fut->state = GET_WHO_SENT;
        else if (ebal.type == CLOSE)
            fut->state = GET_WHO_DISCONNECTED;
        else {
            std::cerr << NAME << SPACE << "Unknown event with type " << static_cast<int>(ebal.type) << std::endl;
            exit(0);
        }
    } else if (fut->state == GET_WHO_CONNECTED) {
        read_bytes(fut->fd, &ebal.who, sizeof(int), true);
        std::cout << NAME << SPACE << "Client#" << ebal.who << " connected" << std::endl;

        sockaddr_in caddr = create_addr(ebal.port, inet_addr("127.0.0.1"));
        int esfd = connect_get_fd(caddr);


        fut->state = GET_TYPE;
        Future &mon = loop->create_future(esfd, client_mon);
        mon.skip_next = false;
        mon.future_data = &ebal;

        ebal.connections[ebal.who] = esfd;

        fut->state = GET_TYPE;
    } else if (fut->state == GET_WHO_DISCONNECTED) {
        read_bytes(fut->fd, &ebal.who, sizeof(int), true);

        close(ebal.connections.at(ebal.who));
        std::cout << NAME << SPACE << "Client#" << ebal.who << " disconnected" << std::endl;
        fut->state = GET_TYPE;
    } else if (fut->state == GET_WHO_SENT) {
        read_bytes(fut->fd, &ebal.who, sizeof(int), true);
        fut->state = GET_LENGTH;
    } else if (fut->state == GET_LENGTH) {
        read_bytes(fut->fd, &ebal.length, sizeof(int), true);
        fut->state = GET_MESSAGE;
    }
}


int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << argv[0] << " usage: " << argv[0] << " [port]" << std::endl;
        return 0;
    }

    unsigned short port = atoi(argv[1]);

    connections_t connections;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    set_reuse(sockfd);
    sockaddr_in addr = create_addr(PROXY_PORT, inet_addr(PROXY_HOST));
    int res = connect(sockfd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));

    if (res < 0) {
        perror("connect()");
        return 0;
    }

    unsigned short rport = retrieve_port(sockfd);

    std::cout << "SOCKFD: " << sockfd << std::endl;

    std::cout << "Proxying 127.0.0.1:" << port << " to " << PROXY_HOST << ":" << rport << "..." << std::endl;

    EBall ball{sockfd, connections, port};

    EventLoop loop;

    Future &msg_fut = loop.create_future(sockfd, on_server_message);
    msg_fut.future_data = &ball;

    loop.run();

    return 0;
}