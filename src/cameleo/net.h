//
// Created by kvxmmu on 3/27/21.
//

#ifndef CAMELEO_NET_H
#define CAMELEO_NET_H

#include <netinet/in.h>

namespace Cameleo {
    class Client {
    public:
        int sockfd;
        sockaddr_in addr{};

        Client(int _sockfd, const sockaddr_in &_addr) : sockfd(_sockfd), addr(_addr) {

        }
    };
}

#endif //CAMELEO_NET_H
