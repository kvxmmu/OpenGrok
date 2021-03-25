#include <iostream>

#include <event_loop.hpp>

#include "opengrok/opengrok.hpp"

int main() {
    GrokLoop loop;
    OpenGrok::MainServer server(8080);

    loop.add_observer(&server);

    loop.run();

    return 0;
}
