#include <iostream>

#include <grokbuffer/wrapper.hpp>
#include <libconfig.h++>

#include <definitions.hpp>
#include <opengrok/protocol_definition.hpp>
#include <opengrok/server/server.hpp>

using namespace libconfig;

int main() {
    Config config;
    config.readFile(CONFIG_FILE_LOCATION);

    int port;
    std::string server_name;
    std::string inet_addr;

    try {
        std::string _srv = config.lookup("OpenGrok.server_name");
        int _prt = config.lookup("OpenGrok.listen_port");
        std::string _inet_addr = config.lookup("OpenGrok.listen_addr");

        server_name = _srv;
        port = _prt;
        inet_addr = _inet_addr;
    } catch (const SettingNotFoundException &nfex) {
        std::cerr << "[OpenGrok:Server] Config parsing error: " << nfex.what() << " by path " << nfex.getPath() << std::endl;

        return 1;
    }


    if (port > 0xffffu) {
        std::cerr << "[OpenGrok:Server] Invalid port: " << port << ", maximum port value is " << 0xffffu << std::endl;

        return 2;
    }

    in_addr inaddr{};
    int response = inet_aton(inet_addr.c_str(), &inaddr);

    std::cout << "Listening at port " << port << std::endl;

    if (response == -1) {
        std::cerr << "[OpenGrok:Server] Invalid IP format for listen address" << std::endl;

        return 3;
    }

    Cameleo::EventLoop loop;

    auto protocol = new GrokBufferProtocol::Server(inaddr.s_addr, port);
    OpenGrok::MainServer main_server(loop, *protocol,
            server_name);

    protocol->error_type = ERROR;
    protocol->too_long_packet_description = TOO_LONG_PACKET_E;
    protocol->too_long_packet_error_code = TOO_LONG_PACKET;

    protocol->packet_callback = [&main_server](BufferReader &reader, int fd, uint8_t type,
            uint32_t length) {
        main_server.on_packet(reader, fd, type, length);
    };
    protocol->disconnect_callback = [&main_server](int fd) {
        main_server.on_disconnect(fd);
    };

    loop.add_observer(protocol);
    loop.run();

    return 0;
}
