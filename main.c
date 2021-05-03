#include <stdio.h>

#define EPOLL_THROW
#include <event_loop/event_loop.h>
#include <lib/platform.h>

#ifdef _PLATFORM_WINDOWS
#include <winsock2.h>
struct WSAData __ws;
#endif

#include <server/server.h>

#define LISTEN_PORT 6567
#define LISTEN_ADDR INADDR_ANY

void start_server(EventLoop *loop, uint16_t listen_port,
        address_t listen_addr, backlog_t backlog)  {
    OpengrokConfig cfg;
    cfg.port = listen_port;
    cfg.address = listen_addr;
    cfg.backlog = backlog;

    LoopObserver main_server;
    main_server.ptr = &cfg;

    observer_init(&main_server, loop,
                  true);
    observer_on_init(&main_server, opengrok_init);

    observer_on_connect(&main_server, opengrok_on_connect);
    observer_on_read(&main_server, opengrok_on_read);
    observer_on_disconnect(&main_server, opengrok_on_disconnect);

    ev_add_observer(loop, &main_server);

    ev_run(loop);

    printf("Server shut down\n");
}

int main() {
#ifdef _PLATFORM_WINDOWS
    WSAStartup(MAKEWORD(1, 1), &__ws);
#endif
    EventLoop loop;

    ev_init(&loop);
    start_server(&loop, LISTEN_PORT,
                 LISTEN_ADDR, 4096);
    ev_free(&loop);

    return 0;
}
