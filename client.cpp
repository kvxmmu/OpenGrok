//
// Created by nero on 12.06.2021.
//

#include <iostream>
#include <loop/loop.hpp>

#include <grokclient/client.hpp>
#include <tclap/CmdLine.h>


bool is_valid_port(int val) {
    return (val > 0) && (val <= 0xffffu);
}


int main(int argc, char *argv[]) {
    try {
        TCLAP::CmdLine cmd("Simple freegrok client example", ' ', FREEGROK_VER);
        TCLAP::ValueArg<std::string> host_arg("t", "host", "FreeGrok host", true, "", "host");
        TCLAP::ValueArg<int> port_arg("p", "port", "Redirect port", true,
                                      0, "port");
        TCLAP::ValueArg<std::string> magic_arg("m", "magic", "FreeGrok auth magic", false,
                                               "", "password");
        TCLAP::ValueArg<int> grok_port("g", "server-port", "FreeGrok server port",
                                       false, 6567, "port");
        TCLAP::ValueArg<std::string> redirect_host("r", "redirect-host", "Redirect host",
                                                   false, "127.0.0.1", "host");
        TCLAP::ValueArg<int> listen_port("l", "listen-port", "Port to open on server side",
                                         false, 1, "port");

        cmd.add(host_arg);
        cmd.add(port_arg);
        cmd.add(magic_arg);
        cmd.add(redirect_host);
        cmd.add(listen_port);

        cmd.parse(argc, argv);
        auto gport = grok_port.getValue();
        auto listen_value = listen_port.getValue();

        if (!is_valid_port(gport) || !is_valid_port(listen_value)) {
            std::cerr << "[FreeGrok] Invalid port" << std::endl;

            return 3;
        }

        ///

        auto port = static_cast<uint16_t>(gport);
        auto listen_port_val = static_cast<uint16_t>(listen_value);
        uint32_t addr = inet_addr(host_arg.getValue().c_str());

        if (addr == static_cast<uint32_t>(-1)) {
            std::cerr << "[FreeGrok] Unexpected address " << host_arg.getValue() << std::endl;

            return 1;
        }

        Loop loop;
        GrokClient client(addr,
                          6567, inet_addr("127.0.0.1"), port,
                          magic_arg.getValue(),
                          listen_port_val);

        loop.add_observer(&client);
        loop.run();

        return 0;
    } catch (TCLAP::ArgException &e) {  // TCLAP fast example, PR if you want to move this try/catch block
        std::cerr << "[FreeGrok] " << e.argId() << " error: " << e.error() << std::endl;

        return 5;
    }
}

