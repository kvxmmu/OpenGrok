#include <iostream>
#include <grok/server.hpp>

#include <csignal>

#define MAX_EVENTS 4096
#define LISTEN_PORT 8089

bool running = true;

int main() {
    Server server;
    server.init(LISTEN_PORT);

    signal(SIGPIPE, SIG_IGN);

    std::cout << NAME << SPACE << "Server started listening " << LISTEN_PORT << " port" << std::endl;

    server.run_until_complete();
    return 0;
}
