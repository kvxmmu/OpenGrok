//
// Created by nero on 5/3/2021.
//

#ifndef OPENGROK_FIFO_H
#define OPENGROK_FIFO_H

/*
 * Basic First in first out queue implementation
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define FIFO_GETOFFSET(index, item_size) ((index) * (item_size))

typedef struct {
    char *raw;

    size_t cursor;
    size_t item_size;

    size_t length;
    size_t capacity;

    size_t resort_cursor;
} Fifo;

static void fifo_init(Fifo *fifo, size_t item_size) {
    fifo->cursor = 0;
    fifo->item_size = item_size;
    fifo->length = 0;
    fifo->capacity = 0;
    fifo->resort_cursor = 100;
}

static void fifo_resize(Fifo *fifo, size_t items) {
    fifo->raw = realloc(fifo->raw, fifo->item_size * items);
    fifo->capacity = items * fifo->item_size;
}

static void fifo_double_size(Fifo *fifo) {
    if (fifo->capacity == 0) {
        fifo_resize(fifo, fifo->item_size << 1u);

        return;
    }

    fifo_resize(fifo, fifo->capacity << 1u);
}

static void fifo_shift(Fifo *fifo, size_t index) {
    size_t offset = FIFO_GETOFFSET(index, fifo->item_size);

    memmove(fifo->raw, fifo->raw+offset,
            fifo->length * fifo->item_size);

    fifo->cursor = 0;
}

static size_t fifo_get_offset(Fifo *fifo, size_t index) {
    size_t start = fifo->cursor * fifo->item_size;
    size_t last_offset = FIFO_GETOFFSET(index, fifo->item_size);
    size_t push_offset = start + last_offset;
}

static void fifo_push(Fifo *fifo, void *ptr) {
    size_t push_offset = fifo_get_offset(fifo, fifo->length);

    while (push_offset > fifo->capacity
           || fifo->capacity == 0) fifo_double_size(fifo);

    memcpy(fifo->raw+push_offset, ptr, fifo->item_size);
    fifo->length++;
}

static void fifo_pop(Fifo *fifo) {
    if (fifo->length == 0) {
        fprintf(stderr, "fifo_pop(): No data in fifo\n");

        abort();
    }

    fifo->cursor++;
    fifo->length--;

    if (fifo->cursor >= fifo->resort_cursor) {
        fifo_shift(fifo, fifo->cursor);
    }
}

static void fifo_get(Fifo *fifo, void *ptr) {
    if (fifo->length == 0) {
        fprintf(stderr, "fifo_get(): No data in fifo\n");

        abort();
    }

    size_t offset = fifo_get_offset(fifo, 0);

    memcpy(ptr, fifo->raw+offset, fifo->item_size);
}

static void fifo_pop_and_get(Fifo *fifo, void *ptr) {
    fifo_get(fifo, ptr);
    fifo_pop(fifo);
}

static void fifo_free(Fifo *fifo) {
    free(fifo->raw);
}

#endif //OPENGROK_FIFO_H
