//
// Created by nero on 5/2/21.
//

#ifndef OPENGROK_EVENT_LOOP_H
#define OPENGROK_EVENT_LOOP_H

struct LoopObserver;

#include <lib/queue.h>
#include <lib/epoll.h>
#include <lib/vector.h>
#include <lib/map.h>

#include "event_bus.h"

#ifndef MAX_EPOLL_SIZE
# define MAX_EPOLL_SIZE 64
#endif

#ifdef _PLATFORM_WINDOWS
#    include <winsock2.h>
typedef int socket_len_t;
#else
#    include <sys/types.h>
#    include <sys/netinet.h>
#    include <sys/socket.h>
#    include <netinet/in.h>
#    include <arpa/inet.h>

void closesocket(socket_t handle) {
    close(handle);
}

typedef socklen_t socket_len_t;

#endif

typedef struct {
    Vector observers;
    EventBus event_bus;

    epoll_t epoll;
    struct epoll_event events[MAX_EPOLL_SIZE];

    bool running;
    int timeout;

    hashmap *send_queue;
    hashmap *mapped_clients;
} EventLoop;

typedef struct LoopObserver LoopObserver;

typedef socket_t (*connect_callback_t)(LoopObserver *observer);
typedef void     (*read_callback_t)(LoopObserver *observer, socket_t handle);
typedef void     (*disconnect_callback_t)(LoopObserver *observer, socket_t handle);
typedef void     (*init_callback_t)(LoopObserver *observer, EventLoop *loop);
typedef void     (*free_callback_t)(LoopObserver *observer);

struct LoopObserver {
    bool is_server;
    EventLoop *loop;
    socket_t handle;

    connect_callback_t    on_connect;
    read_callback_t       on_read;
    disconnect_callback_t on_disconnect;
    init_callback_t       init;
    free_callback_t       on_free;

    void *ptr;
    Vector clients;
};

/// observer

static void observer_init(LoopObserver *observer,
                   EventLoop *loop, bool is_server) {
    vector_init(&observer->clients, sizeof(socket_t));

    observer->loop = loop;
    observer->init = NULL;
    observer->on_free = NULL;
    observer->is_server = is_server;

    observer->handle = socket(AF_INET, SOCK_STREAM, 0);
}

static void observer_add_client(LoopObserver *observer, socket_t handle) {
    vector_append(&observer->clients, &handle);
}

static void observer_remove_client(LoopObserver *observer, socket_t handle) {
    vector_remove(&observer->clients, &handle);
}

static void observer_on_init(LoopObserver *observer, init_callback_t initializer) {
    observer->init = initializer;
}

static void observer_on_free(LoopObserver *observer, free_callback_t deinitializer) {
    observer->on_free = deinitializer;
}

static void observer_on_connect(LoopObserver *observer, connect_callback_t handler) {
    observer->on_connect = handler;
}

static void observer_on_read(LoopObserver *observer, read_callback_t handler) {
    observer->on_read = handler;
}

static void observer_on_disconnect(LoopObserver *observer, disconnect_callback_t handler) {
    observer->on_disconnect = handler;
}

static void observer_free(LoopObserver *observer) {
    vector_free(&observer->clients);
    observer->init = NULL;
    observer->on_connect = NULL;
    observer->on_read = NULL;
    observer->on_disconnect = NULL;
}

/// event loop

static void ev_init(EventLoop *loop) {
    loop->epoll = epoll_create(MAX_EPOLL_SIZE);
    loop->running = false;
    loop->timeout = -1; // no timeout
    loop->send_queue = hashmap_create();
    loop->mapped_clients = hashmap_create();

    vector_init(&loop->observers, sizeof(LoopObserver));
    event_bus_init(&loop->event_bus);
}

void   ev_add_observer   (EventLoop *loop, LoopObserver *observer);
void   ev_remove_observer(EventLoop *loop, LoopObserver *observer);
size_t ev_has_observer   (EventLoop *loop, socket_t handle);

socket_t ev_accept(socket_t handle, struct sockaddr_in *addr);
int      ev_recv(socket_t handle, char *buffer, size_t length);
int      ev_send(socket_t handle, char *buffer, size_t length);
void     ev_internal_disconnect(EventLoop *loop, LoopObserver *observer,
                                socket_t handle);
bool     ev_is_disconnected(socket_t handle);
void     ev_map_client(EventLoop *loop, socket_t handle,
                       LoopObserver *observer);
void     ev_unmap_client(EventLoop *loop, socket_t handle);
bool     ev_is_mapped(EventLoop *loop, socket_t handle);

LoopObserver *ev_get_mapped_client(EventLoop *loop, socket_t handle);

void ev_run(EventLoop *loop);

static void ev_free(EventLoop *loop) {
    vector_free(&loop->observers);
    event_bus_free(&loop->event_bus);
    close_epoll(loop->epoll);

    hashmap_free(loop->send_queue);
    hashmap_free(loop->mapped_clients);
}

#endif //OPENGROK_EVENT_LOOP_H
