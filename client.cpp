#include <grok/evloop.hpp>
#include <iostream>

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << argv[0] << " usage: " << argv[0] << " [port]" << std::endl;
        return 0;
    }

    EventLoop loop;

    return 0;
}