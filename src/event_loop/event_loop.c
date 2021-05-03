//
// Created by nero on 5/2/21.
//

#include "event_loop.h"

socket_t ev_accept(socket_t handle, struct sockaddr_in *addr) {
    socket_len_t len = sizeof(struct sockaddr_in);

    socket_t client_handle = accept(handle, (struct sockaddr *)addr,
            &len);

    return client_handle;
}

int ev_recv(socket_t handle, char *buffer, size_t length) {
    int flags = 0;

#ifndef _PLATFORM_WINDOWS
    flags = MSG_NOSIGNAL;
#endif

    int received = recv(handle, buffer, (int)length, flags);

    if (received < 0) {
        perror("ev_recv()");
    }

    return received;
}

int ev_send(socket_t handle, char *buffer, size_t length) {
    int flags = 0;

#ifndef _PLATFORM_WINDOWS
    flags = MSG_NOSIGNAL;
#endif

    int sent = send(handle, buffer, (int)length, flags);

    if (sent < 0) {
        perror("ev_send()");
    }

    return sent;
}

void ev_add_observer(EventLoop *loop, LoopObserver *observer) {
    observer->init(observer, loop);

    epoll_add(loop->epoll,
              observer->handle, EPOLLIN);

    vector_append(&loop->observers,
                  observer);
}

void ev_remove_observer(EventLoop *loop, LoopObserver *observer) {
    for (size_t pos = 0; pos < vector_size(&observer->clients); pos++) {
        socket_t handle = *(socket_t *)vector_buffer(&observer->clients, pos);

        ev_internal_disconnect(loop, observer,
                               handle);
    }

    vector_free(&observer->clients);
    vector_remove(&loop->observers, &observer);
}

size_t ev_has_observer(EventLoop *loop, socket_t handle) {
    for (size_t pos = 0; pos < vector_size(&loop->observers); pos++) {
        LoopObserver *observer = vector_buffer(&loop->observers, pos);

        if (observer->handle == handle) {
            return pos;
        }
    }

    return N_POS;
}

bool ev_is_disconnected(socket_t handle) {
    char buf;
    int flags = 0;

#ifndef _PLATFORM_WINDOWS
    flags = MSG_NOSIGNAL;
#endif

    int ret = recv(handle, &buf, sizeof(char), MSG_PEEK | flags);

    return ret <= 0;
}

void ev_internal_disconnect(EventLoop *loop, LoopObserver *observer,
                            socket_t handle) {
    closesocket(handle);
    observer_remove_client(observer, handle);

    hashmap_remove(loop->mapped_clients, &handle, sizeof(socket_t));
}

LoopObserver *ev_get_mapped_client(EventLoop *loop, socket_t handle) {
    LoopObserver *linked_observer;
    uintptr_t ptr;

    if (!hashmap_get(loop->mapped_clients, &handle, sizeof(socket_t),
                     &ptr)) {
        fprintf(stderr, "ev_get_mapped_client(): No such client\n");

        abort();
    }

    memcpy(&linked_observer, &ptr, sizeof(uintptr_t));

    return linked_observer;
}

void ev_map_client(EventLoop *loop, socket_t handle,
                       LoopObserver *observer) {
    uintptr_t uintptr;

    memcpy(&uintptr, &observer, sizeof(uintptr_t));

    hashmap_set(loop->mapped_clients, &handle,
                sizeof(socket_t), uintptr);
}

bool ev_is_mapped(EventLoop *loop, socket_t handle) {
    uintptr_t uintptr;

    return hashmap_get(loop->mapped_clients, &handle,
                       sizeof(socket_t), &uintptr);
}

void ev_run(EventLoop *loop) {
    loop->running = true;

    while (loop->running) {
        if (vector_empty(&loop->observers)) {
            break;
        }

        int nfds = epoll_wait(loop->epoll,
                              loop->events, MAX_EPOLL_SIZE, loop->timeout);

        if (nfds < 0) {
            fprintf(stderr, "ev_run(): epoll_wait error");

            abort();
        }

        for (size_t pos = 0; pos < (size_t)nfds; pos++) {
            struct epoll_event *event = &loop->events[pos];
            socket_t handle = event->data.u64;

            if (event->events & EPOLLOUT) {
                
            } else if (event->events & EPOLLHUP) {
                size_t obs_pos;

                if (ev_is_mapped(loop, handle)) {
                    LoopObserver *observer = ev_get_mapped_client(loop, handle);
                    observer->on_disconnect(observer, handle);

                    ev_internal_disconnect(loop, observer,
                                           handle);

                    continue;
                } else if ((obs_pos = ev_has_observer(loop, handle)) != N_POS) {
                    LoopObserver *observer = vector_buffer(&loop->observers,
                                                           obs_pos);
                    observer->on_disconnect(observer, handle);
                    ev_remove_observer(loop, observer);

                    continue;
                }

                fprintf(stderr, "Unknown error\n");
                abort();
            } else if (event->events & EPOLLIN) {
                size_t obs_pos;

                if ((obs_pos = ev_has_observer(loop, handle)) != N_POS) {
                    LoopObserver *observer = vector_buffer(&loop->observers, obs_pos);

                    if (observer->is_server) {
                        socket_t client_handle = observer->on_connect(observer);
                        epoll_add(loop->epoll, client_handle, EPOLLIN | EPOLLHUP);

                        ev_map_client(loop, client_handle,
                                      observer);

                        continue;
                    }

                    observer->on_read(observer, handle);
                } else {
                    LoopObserver *linked_observer = ev_get_mapped_client(loop, handle);
                    linked_observer->on_read(linked_observer, handle);

                    continue;
                }
            }
        }
    }
}


