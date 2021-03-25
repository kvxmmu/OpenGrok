//
// Created by kvxmmu on 3/25/21.
//

#include "event_loop.hpp"


//// SendQueue

bool SendQueue::perform(int fd) {
    if (!this->is_queue_exists(fd)) {
        return;
    }

    auto &queue = this->get_queue(fd);

    if (queue.empty()) {
        this->queues.erase(fd);

        printf("[GrokLoop:SendQueue] Queue is empty\n");

        return;
    }

    auto &front = queue.front();

    int bytes_read = read_bytes(fd, front.buffer+front.received,
            front.length - front.received);

    if (bytes_read < 0) {
        front.received = front.length;

        perror("SendQueue::perform()");
    } else {
        front.received += bytes_read;
    }

    bool done = front.received >= front.length;
    bool is_empty = false;

    if (done) {
        this->allocator->deallocate(front.buffer, front.length);

        queue.pop_front();
        is_empty = this->remove_if_empty(queue, fd);
    }

    return is_empty;
}

void SendQueue::clear_queue(int fd) {
    if (!this->is_queue_exists(fd)) {
        return;
    }

    auto &queue = this->get_queue(fd);

    for (auto &item : queue) {
        this->allocator->deallocate(item.buffer, item.length);
    }

    queue.clear();
    this->queues.erase(fd);
}

void SendQueue::push(int fd, char *buffer, size_t length) {
    SendItem item(this->allocator->allocate(length), length);
    memcpy(item.buffer, buffer, length);

    if (!this->is_queue_exists(fd)) {
        this->queues[fd] = sendqueue_t();
    }

    auto &queue = this->get_queue(fd);

    queue.push_back(item);
}

void GrokLoop::run() {
    this->running = true;
}
