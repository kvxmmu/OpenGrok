//
// Created by nero on 12.06.2021.
//

#include <iostream>
#include <loop/loop.hpp>

#include <grokclient/client.hpp>


int main() {
    uint16_t port;
    std::string ip;

    std::cout << "Enter port: ";
    std::cin >> port;
    std::cout << "Enter freegrok host: ";
    std::cin >> ip;

    uint32_t addr = inet_addr(ip.c_str());

    if (addr == static_cast<uint32_t>(-1)) {
        std::cout << "Unexpected address" << std::endl;

        return 1;
    }

    Loop loop;
    GrokClient client(addr,
                      6567, inet_addr("127.0.0.1"), port);

    loop.add_observer(&client);
    loop.run();

    return 0;
}

