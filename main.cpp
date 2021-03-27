#include <iostream>

#include <grokbuffer/wrapper.hpp>
#include <libconfig.h++>

#include <definitions.hpp>
#include <opengrok/protocol_definition.hpp>
#include <opengrok/server/server.hpp>

using namespace libconfig;

int main() {
    Config config;
    config.readFile("../config/opengrok.cfg");

    const Setting &root = config.getRoot();
    auto &settings = root["OpenGrok"];

    int port;
    std::string server_name;
    std::string inet_addr;

    try {
        root.lookupValue("server_name", server_name);
        root.lookupValue("listen_port", port);
        root.lookupValue("listen_addr", inet_addr);
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

    if (response == -1) {
        std::cerr << "[OpenGrok:Server] Invalid IP format for listen address" << std::endl;

        return 3;
    }

    Cameleo::EventLoop loop;

    GrokBufferProtocol::Server protocol(inaddr.s_addr, port);
    OpenGrok::MainServer main_server(loop, protocol);

    protocol.error_type = ERROR;
    protocol.too_long_packet_description = TOO_LONG_PACKET_E;
    protocol.too_long_packet_error_code = TOO_LONG_PACKET;

    loop.add_observer(&protocol);
    loop.run();

    return 0;
}
