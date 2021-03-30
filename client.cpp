//
// Created by kvxmmu on 3/28/21.
//

#include <iostream>

#include <opengrok/client/client.hpp>
#include <grokbuffer/wrapper.hpp>

#include <unistd.h>

int main(int argc, char **argv) {
    int port;
    std::string host;

    if (argc < 5) {
        std::cerr << "OpenGrok:Client Help. "
                     "Required arguments:\n"
                     "\t-p [port_number]\tLocal port\n"
                     "\t-h [opengrok host]\tOpenGrok server host"<< std::endl;

        return 0;
    }

    int opt;

    while ((opt = getopt(argc, argv, "p:h:")) != -1) {
        switch (opt) {
            case 'p':
                port = std::stoi(optarg);

                break;

            case 'h': {
                host = std::string(optarg);

                break;
            }

            case '?':
                return 1;

            default:
                break;
        }
    }

    if (port < 0 || port > 0xffffu) {
        std::cerr << argv[0] << " Error: Port has an invalid option" << std::endl;

        return 2;
    }

    sockaddr_in ctest{};
    opt = inet_aton(host.c_str(), &ctest.sin_addr);

    if (opt == -1) {
        std::cerr << argv[0] << " Error: Host has an invalid option" << std::endl;

        return 3;
    }

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