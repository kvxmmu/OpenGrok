//
// Created by nero on 12.06.2021.
//

#include <iostream>
#include <loop/loop.hpp>

#include <grokclient/client.hpp>


int main() {
    uint16_t port;

    std::cout << "Enter port: ";
    std::cin >> port;

    Loop loop;
    GrokClient client(inet_addr("127.0.0.1"),
                      6567, inet_addr("127.0.0.1"), port);

    loop.add_observer(&client);
    loop.run();
}

