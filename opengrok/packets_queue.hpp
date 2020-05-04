//
// Created by kvxmmu on 5/3/20.
//

#ifndef OPENGROK2_PACKETSQUEUE_HPP
#define OPENGROK2_PACKETSQUEUE_HPP

#include <unordered_map>
#include <deque>
#include <unistd.h>
#include <cstring>


#include <opengrok/def.hpp>


struct QueueItem {
    int fd = 0;

    int sent = 0;
    int length = 0;

    char buffer[BUFF_SIZE]{};

    void *extended = nullptr;

    void (*on_progress)(QueueItem *item) = nullptr;
};


class PacketsQueue {
public:
    std::unordered_map<int, std::deque<QueueItem *>> queues;

    void check_emptiness(int fd);

    void push(int fd, char *pkt, int size, void (*on_progress)(QueueItem *item) = nullptr, void *data = nullptr);

    void push_if(int fd, QueueItem *item);

    bool check(int fd, bool &end);
};

class UsagePool {
public:
    std::unordered_map<int, bool> usage;

    bool is_used(int fd);
    void set_used(int fd, bool state);
    void clear_used(int fd);
};


#endif //OPENGROK2_PACKETSQUEUE_HPP
