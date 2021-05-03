//
// Created by nero on 5/3/2021.
//

#include "server.h"

socket_t opengrok_on_connect(LoopObserver *observer) {
    struct sockaddr_in addr;
    socket_t client_handle = ev_accept(observer->handle, &addr);

    printf("Connected handle %zu\n", client_handle);

    return client_handle;
}

void opengrok_on_read(LoopObserver *observer, socket_t handle) {
    char buf[4097];

    int received = ev_recv(handle, buf, 4096);

    printf("Data: %s; Received: %d\n", buf, received);
}

void opengrok_on_disconnect(LoopObserver *observer, socket_t handle) {
    printf("Disconnected handle %zu\n", handle);
}