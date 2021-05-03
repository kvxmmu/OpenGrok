//
// Created by nero on 5/3/2021.
//

#ifndef OPENGROK_SERVER_H
#define OPENGROK_SERVER_H

#include <event_loop/event_loop.h>
#include <lib/map.h>

#include <grokbuffer/protocol.h>

typedef int backlog_t;

typedef struct {
    uint16_t port;
    address_t address;

    backlog_t backlog;

    hashmap *clients;
} OpengrokConfig;

typedef struct {

} OpengrokProxyServer;

typedef struct {
    GBufPacket packet;
    OpengrokProxyServer *proxy;
} OpengrokClient;

static void opengrok_init(LoopObserver *observer, EventLoop *loop) {
    OpengrokConfig *cfg = observer->ptr;

    struct sockaddr_in addr;
    addr.sin_addr.s_addr = cfg->address;
    addr.sin_port = htons(cfg->port);
    addr.sin_family = AF_INET;

    optlen_t y = 1;
    setsockopt(observer->handle, SOL_SOCKET, SO_REUSEADDR, (void *)&y, sizeof(optlen_t));

    if (bind(observer->handle, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        perror("bind()");
        abort();
    }

    if (listen(observer->handle, cfg->backlog) != 0) {
        perror("listen()");
        abort();
    }

    fprintf(stdout, "[OpenGrok:Server] Listening on port %d\n", cfg->port);
    fflush(stderr);
}

socket_t opengrok_on_connect(LoopObserver *observer);
void     opengrok_on_read(LoopObserver *observer, socket_t handle);
void     opengrok_on_disconnect(LoopObserver *observer, socket_t handle);

#endif //OPENGROK_SERVER_H
