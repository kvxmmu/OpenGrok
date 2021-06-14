#include <iostream>

#include <loop/loop.hpp>
#include <grokserver/server.hpp>

int main() {
    Config cfg(6567, INADDR_ANY,
               "FreeGrok/1.1a");
    FreeGrok server(cfg);
    Loop loop;

    loop.add_observer(&server);
    loop.run();

    return 0;
}
