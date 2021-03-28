//
// Created by kvxmmu on 3/28/21.
//

#include <iostream>

#include <opengrok/client/client.hpp>
#include <grokbuffer/wrapper.hpp>

int main() {
    int port;
    std::string host;

    std::cout << "Enter port: ";
    std::cin >> port;

    std::cout << "Enter host: ";
    std::cin >> host;

    Cameleo::EventLoop loop;
    auto protocol = new GrokBufferProtocol::Client(host.c_str(), 6567);
    OpenGrok::Client client("127.0.0.1", port,
            loop, *protocol);

    protocol->packet_callback = [&client](BufferReader &reader, int fd,
            uint8_t type, uint32_t length) {
        client.on_packet(reader, fd, type, length);
    };
    protocol->motd_callback = [&client](int fd) {
        client.on_motd(fd);
    };
    protocol->disconnect_callback = [&client](int fd) {
        client.on_disconnect(fd);
    };

    loop.add_observer(protocol);
    loop.run();

    return 0;
}