//
// Created by nero on 5/2/21.
//

#ifndef OPENGROK_VECTOR_H
#define OPENGROK_VECTOR_H

#include <stdbool.h>
#include <stddef.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "libdef.h"

#define V_DEFAULT_ITEMS_CAP 10
#define V_GETBUF_OFFSET(index, item_size) ((index) * (item_size))

typedef struct {
    char *raw;

    size_t item_size;
    size_t capacity;
    size_t length;
} Vector;

static void vector_init(Vector *vector, size_t item_size) {
    vector->capacity = V_DEFAULT_ITEMS_CAP * item_size;
    vector->raw = calloc(V_DEFAULT_ITEMS_CAP, item_size);
    vector->length = 0;
    vector->item_size = item_size;
}

static size_t vector_size(Vector *vector) {
    return vector->length;
}

static void vector_shift(Vector *vector, size_t index) {
    if (index+1 >= vector->length) {
        fprintf(stderr, "vector_shift(): Out of range\n");

        abort();
    }

    size_t shift_count = vector->length - index - 1;
    size_t shifted = 0;
    size_t pos = index;

    while (shifted < shift_count) {
        memcpy(vector->raw+ V_GETBUF_OFFSET(pos, vector->item_size),
                vector->raw+V_GETBUF_OFFSET(pos+1, vector->item_size), vector->item_size);

        pos++;
        shifted++;
    }
}

static bool vector_empty(Vector *vector) {
    return vector->length == 0;
}

static void vector_set(Vector *vector, size_t index,
                char c) {
    if (vector_size(vector) <= index) {
        abort();
    }

    char *buf = vector->raw + V_GETBUF_OFFSET(index, vector->item_size);
    memset(buf, c, vector->item_size);
}

static void vector_get(Vector *vector, size_t index,
                void *dest) {
    if (vector->length <= index) {
        fprintf(stderr, "vector_get(): Out of range\n");
        abort();
    }

    char *buf = vector->raw + V_GETBUF_OFFSET(index, vector->item_size);
    memcpy(dest, buf, vector->item_size);
}

static void *vector_buffer(Vector *vector, size_t index) {
    if (vector->length <= index) {
        fprintf(stderr, "vector_buffer(): Out of range(%zu)\n", index);

        abort();
    }

    return vector->raw+V_GETBUF_OFFSET(index, vector->item_size);
} // get data without copying to buffer

static void vector_pop(Vector *vector, size_t index) {
    if (vector->length <= index) {
        fprintf(stderr, "vector_pop(): Out of range\n");
    }

    vector_shift(vector, index);
    vector->length--;
}

static void vector_resize(Vector *vector, size_t items) {
    size_t capacity = vector->item_size * items;

    vector->raw = realloc(vector->raw, capacity);
    vector->capacity = capacity;
}

static void vector_double_capacity(Vector *vector) {
    if (vector->capacity == 0) {
        vector_resize(vector, vector->item_size);

        return;
    }

    vector_resize(vector, vector->capacity << 1u);
}

static void vector_append(Vector *vector, void *src) {
    size_t offset = vector->length * vector->item_size;

    while (vector->capacity < offset) {
        vector_double_capacity(vector);
    }

    memcpy(vector->raw+offset, src, vector->item_size);
    vector->length++;
}

static void vector_remove(Vector *vector, void *item) {
    if (vector_empty(vector)) {
        return;
    }

    size_t index = N_POS;

    for (size_t pos = 0; pos < vector->length; pos++) {
        size_t offset = V_GETBUF_OFFSET(pos, vector->item_size);

        if (memcmp(vector->raw+offset, item, vector->item_size) == 0) {
            index = pos;

            break;
        }
    }

    if (index == N_POS) {
        fprintf(stderr, "vector_remove(): No such item\n");

        abort();
    }

    vector_pop(vector, index);
}

static size_t vector_in(Vector *vector, void *item) {
    for (size_t pos = 0; pos < vector->length; pos++) {
        size_t offset = V_GETBUF_OFFSET(pos, vector->item_size);

        if (memcmp(item, vector->raw+offset,
                   vector->item_size) == 0) {
            return pos;
        }
    }

    return N_POS;
}

static void vector_free(Vector *vector) {
    vector->item_size = 0;
    vector->capacity = 0;
    vector->length = 0;

    free(vector->raw);
}

#endif //OPENGROK_VECTOR_H
