//
// Created by kvxmmu on 3/25/21.
//

#include "client/client_observer.hpp"

int main() {
    GrokLoop loop;
    ClientObserver client(8080, "127.0.0.1",
            "127.0.0.1", 2280);

    loop.add_observer(&client);
    loop.run();

    return 0;
}