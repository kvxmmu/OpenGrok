//
// Created by kvxmmu on 5/4/20.
//

#include <iostream>
#include <client/clients.hpp>

unsigned short get_connport(int fd) {
    uint8_t useless;
    unsigned short port;

    read_bytes(fd, &useless, sizeof(uint8_t));
    read_bytes(fd, &port, sizeof(unsigned short));

    return port;
}

int main(int argc, char **argv) {

    if (argc < 2) {
        std::cout << NAME << SPACE << argv[0] << SPACE << "usage: " << argv[0] << " [port]" << std::endl;
        return 0;
    }

    unsigned short eport = atoi(argv[1]);

    ProxyClient client(PROXY_PORT, inet_addr(PROXY_HOST));
    client.connect_port = eport;

    int res = connect(client.sockfd, reinterpret_cast<sockaddr *>(&client.addr), sizeof(client.addr));

    if (res < 0) {
        perror("connect()");
        return 0;
    }

    unsigned short rport = get_connport(client.sockfd);

    AbstractProtocol *protocol = &client;
    std::cout << NAME << SPACE << "Listening port " << rport << std::endl;

    EventLoop loop(protocol);

    loop.run();
    return 0;
}