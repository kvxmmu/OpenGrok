#include <iostream>

#include <servers.hpp>
#include <csignal>

#ifdef WIN32
# include <windows.h>
// maybe this will work
#endif

#define PROXY_PORT static_cast<unsigned short>(8091)



int main() {
    signal(SIGPIPE, SIG_IGN);

    std::cout << NAME << SPACE << "Listening on the 0.0.0.0:" << PROXY_PORT << "..." << std::endl;

    ProxyServer server(PROXY_PORT);

    EventLoop loop(reinterpret_cast<AbstractProtocol *>(&server));

    loop.run();
    return 0;
}
