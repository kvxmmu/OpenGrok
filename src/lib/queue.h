//
// Created by nero on 5/2/21.
//

#ifndef OPENGROK_QUEUE_H
#define OPENGROK_QUEUE_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <stdbool.h>

#define Q_DEFAULT_ITEMS_COUNT 5
#define Q_GET_BUFINDEX(index, item_size) ((index) * (item_size) - (item_size))
#define Q_GET_BUFINDEX_FUT(index, item_size) ((index) * (item_size))

typedef struct {
    char *raw;

    size_t capacity;
    size_t length;

    size_t item_size;

    // additional values

    bool set_zeros;
} Queue;

static void queue_init(Queue *queue, size_t item_size) {
    queue->raw = calloc(item_size, item_size);
    queue->length = 0;
    queue->item_size = item_size;
    queue->capacity = Q_DEFAULT_ITEMS_COUNT*item_size;
    queue->set_zeros = false;
}

static void queue_zeros(Queue *queue, bool zeros) {
    queue->set_zeros = zeros;
}

static void queue_resize(Queue *queue, size_t to_capacity) {
    queue->raw = realloc(queue->raw, queue->item_size * to_capacity);
    queue->capacity = to_capacity;
}

static size_t queue_available_items(Queue *queue) {
    return (queue->capacity / queue->item_size) - queue->length;
}

static void queue_double_capacity(Queue *queue) {
    if (queue->capacity == 0) {
        queue_resize(queue, queue->item_size * Q_DEFAULT_ITEMS_COUNT);
    }

    queue_resize(queue, queue->capacity << 1u);
}

static size_t queue_size(Queue *queue) {
    return queue->length;
}

static bool queue_empty(Queue *queue) {
    return queue->length == 0;
}

static void queue_push(Queue *queue, void *item) {
    while (queue->capacity < (queue->item_size*queue->length+queue->item_size)) {
        queue_double_capacity(queue);
    }

    memcpy(queue->raw + Q_GET_BUFINDEX_FUT(queue->length, queue->item_size), item,
           queue->item_size);
    queue->length++;
}

static void queue_set(Queue *queue, char c,
               size_t index) {
    memset(queue->raw+Q_GET_BUFINDEX(index, queue->item_size), c, queue->item_size);
}

static void queue_pop(Queue *queue) {
    if (queue->length == 0) {
        abort();
    }

    if (queue->set_zeros) queue_set(queue, 0, queue->length);

    queue->length--;
}

static void queue_get_index(Queue *queue, void *dest,
                     size_t index, bool *status) {
    if (queue->length == 0) {
        if (status != NULL) *status = false;
    }

    if (status != NULL) *status = true;

    size_t item_index = Q_GET_BUFINDEX_FUT(index, queue->item_size);

    memcpy(dest, queue->raw+item_index,
           queue->item_size);
}

static void queue_get(Queue *queue, void *dest,
               bool *status) {
    if (queue_empty(queue)) {
        abort();
    }

    queue_get_index(queue, dest,
                    queue->length - 1, status);
}

static void queue_pop_and_get(Queue *queue, void *dest,
                       bool *status) {
    queue_get(queue, dest, status);
    queue_pop(queue);
}

static void queue_free(Queue *queue) {
    free(queue->raw);
    queue->capacity = 0;
    queue->length = 0;
    queue->raw = NULL;
}

#endif //OPENGROK_QUEUE_H
