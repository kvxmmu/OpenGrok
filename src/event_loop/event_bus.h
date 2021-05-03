//
// Created by nero on 5/2/21.
//

#ifndef OPENGROK_EVENT_BUS_H
#define OPENGROK_EVENT_BUS_H

#include <lib/queue.h>
#include <lib/epoll.h>

typedef struct {
    Queue queue;
} EventBus;

typedef enum {
    DISCONNECT,
} BusEventType;

typedef struct {
    BusEventType type;

    socket_t handle;
} BusEvent;

static void event_bus_init(EventBus *bus) {
    queue_init(&bus->queue, sizeof(BusEvent));
}

static bool event_bus_empty(EventBus *bus) {
    return queue_empty(&bus->queue);
}

static size_t event_bus_size(EventBus *bus) {
    return queue_size(&bus->queue);
}

static void event_bus_pop(EventBus *bus, BusEvent *ptr) {
    bool status;

    queue_pop_and_get(&bus->queue, ptr, &status);

    if (!status) {
        fprintf(stderr, "event_bus_pop(): No events");

        abort();
    }
}

static void event_bus_free(EventBus *bus) {
    queue_free(&bus->queue);
}

#endif //OPENGROK_EVENT_BUS_H
