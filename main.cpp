#include <iostream>

#include <loop/loop.hpp>
#include <grokserver/server.hpp>
#include <grokserver/http_interface.hpp>

#include <csignal>
#include <freegrok/inih.hpp>
#include <freegrok/defaults.hpp>
#include <grokserver/magic_generator.hpp>
#define MAX_MAGIC_SIZE 1024


static void sighup_handler(int) {
    // TODO: add config file re-reading
}


int main() {
#ifdef SIGPIPE
    signal(SIGPIPE, SIG_IGN);
#endif

#ifdef SIGHUP
    signal(SIGHUP, sighup_handler);
#endif

    INIReader reader(CONFIG_FILE_LOCATION);

    if (reader.ParseError() != 0) {
        std::cerr << "[FreeGrok] Config file parse error" << std::endl;

        return 1;
    }

    auto port = reader.GetInteger(FREEGROK_MAIN_SECT, FREEGROK_PORT_FIELD,
                                  DEFAULT_LISTEN_PORT);
    auto magic = reader.Get(FREEGROK_MAIN_SECT, FREEGROK_MAGIC_FIELD,
                            "");
    auto http_port = reader.GetInteger(FREEGROK_MAIN_SECT, FREEGROK_HTTP_PORT_FIELD,
                                       8080);

    if (magic.size() >= MAX_MAGIC_SIZE) {
        std::cerr << "[FreeGrok] Too long magic field" << std::endl;

        return 3;
    } else if (port < 0 || port > 0xffffu) {
        std::cerr << "[FreeGrok] Invalid port value" << std::endl;

        return 2;
    } else if (http_port < 0 || port > 0xffffu) {
        std::cerr << "[FreeGrok] Invalid http port value" << std::endl;

        return 4;
    }

    char magic_buf[MAX_MAGIC_SIZE];
    auto magic_len = generate_magic(magic_buf, MAX_MAGIC_SIZE,
                                    magic);

    std::cout << "[FreeGrok] Generated magic: " << std::string(magic_buf, magic_len) << std::endl;

    Config cfg(port, INADDR_ANY,
               reader.Get(FREEGROK_MAIN_SECT, DEFAULT_SERVER_NAME,
                          DEFAULT_SERVER_NAME),
                          http_port);
    Loop loop;

    if (cfg.http_port != 0) {
        HttpInterface iface(cfg.http_port);
        loop.add_observer(&iface);
    }

    FreeGrok server(cfg, magic_buf,
                    magic_len);

    loop.add_observer(&server);
    loop.run();

    return 0;
}
