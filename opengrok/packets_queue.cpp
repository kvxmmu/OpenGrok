//
// Created by kvxmmu on 5/3/20.
//

#include <iostream>
#include "packets_queue.hpp"

void PacketsQueue::check_emptiness(int fd) {
    if (this->queues.find(fd) == this->queues.end())
        return;
    std::deque<QueueItem *> &qitems = this->queues.at(fd);
    if (!qitems.empty())
        this->queues.erase(fd);
}

void PacketsQueue::push(int fd, char *pkt, int size, void (*on_progress)(QueueItem *), void *data) {
    auto item = new QueueItem;
    item->fd = fd;
    item->sent = 0;
    item->on_progress = on_progress;
    item->length = size;
    item->extended = data;

    memmove(item->buffer, pkt, size);

    this->push_if(fd, item);
}

void PacketsQueue::push_if(int fd, QueueItem *item) {
    if (this->queues.find(fd) == this->queues.end())
        this->queues.emplace(std::make_pair(fd, std::deque<QueueItem *>()));

    std::deque<QueueItem *> &qitems = this->queues.at(fd);
    qitems.push_back(item);
}

bool PacketsQueue::check(int fd, bool &end) {
    if (this->queues.find(fd) == this->queues.end())
        return false;

    std::deque<QueueItem *> &qitems = this->queues.at(fd);
    if (qitems.empty()) {
        this->queues.erase(fd);
        return false;
    }

    QueueItem *front = qitems.front();

    auto sent = static_cast<size_t>(front->sent);
    auto length = static_cast<size_t>(front->length);

    front->sent += static_cast<int>(write_bytes(front->fd, front->buffer+sent, length-sent));

    if (front->on_progress != nullptr)
        front->on_progress(front);
    if (front->sent >= front->length) {
        qitems.pop_front();
        end = true;
    } else
        end = false;

    return true;
}

// UsagePool

bool UsagePool::is_used(int fd) {
    if (this->usage.find(fd) == this->usage.end())
        return false;
    return this->usage.at(fd);
}

void UsagePool::set_used(int fd, bool state) {
    if (this->usage.find(fd) == this->usage.end()) {
        this->usage.emplace(std::make_pair(fd, state));
        return;
    }

    this->usage[fd] = state;
}

void UsagePool::clear_used(int fd) {
    if (this->usage.find(fd) != this->usage.end())
        this->usage.erase(fd);
}
