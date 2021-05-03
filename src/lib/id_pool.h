//
// Created by nero on 5/2/21.
//

#ifndef OPENGROK_ID_POOL_H
#define OPENGROK_ID_POOL_H

#include "queue.h"

#ifndef ID_TYPE
# define ID_TYPE size_t
#endif

typedef struct {
    ID_TYPE last_id;

    Queue queue;
} IDPool;

static void idpool_init(IDPool *pool) {
    pool->last_id = 0;

    queue_init(&pool->queue, sizeof(ID_TYPE));
}

static void idpool_remove(IDPool *pool, ID_TYPE id) {
    queue_push(&pool->queue, &id);
}

static ID_TYPE idpool_get_id(IDPool *pool) {
    if (queue_empty(&pool->queue)) {
        return pool->last_id++;
    }

    ID_TYPE id;
    bool status;

    queue_get(&pool->queue, &id, &status);

    if (!status) abort();
    else queue_pop(&pool->queue);

    return id;
}

static void idpool_free(IDPool *pool) {
    queue_free(&pool->queue);
}

#endif //OPENGROK_ID_POOL_H
