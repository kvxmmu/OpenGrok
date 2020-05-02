#include <iostream>

#include <grok/server.hpp>
#include <csignal>


#define LISTEN_PORT 8089

int main() {
    signal(SIGPIPE, SIG_IGN);
    std::ios::sync_with_stdio(false);

    std::cout << NAME << SPACE << "Start listening on " << LISTEN_PORT << " port" << std::endl;
    Server server(LISTEN_PORT);

    server.poll();

    return 0;
}