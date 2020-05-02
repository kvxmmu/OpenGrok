//
// Created by kvxmmu on 5/1/20.
//

#ifndef OPENGROK_LOOP_HPP
#define OPENGROK_LOOP_HPP

#include <sys/types.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>

#include <deque>
#include <ctime>
#include <vector>

#include <grok/def.hpp>
#include <grok/utils.hpp>

#include <unordered_map>

#include <cstring>
#include <string>

#ifndef MAX_EPOLL_SIZE
# define MAX_EPOLL_SIZE 8192
#endif

#ifndef TIMEOUT
# define TIMEOUT 5000
#endif
#define JUST_REMOVE 65530


typedef uint16_t state_t;


class EventLoop;


class Future {
public:
    int fd;
    int events = EPOLLIN;
    EventLoop *loop = nullptr;

    void *future_data = nullptr;
    state_t state = 0;
    time_t last_active = 0;

    void (*on_finish)(EventLoop *, Future *) = nullptr;
    void (*on_tick)(EventLoop *, Future *);

    bool skip_next = false;


    Future(int _fd, void (*_on_tick)(EventLoop *fut, Future *));

    void update_activity();
    [[nodiscard]] time_t get_time_delta() const;
};

void set_nonblock(int fd);


class EpollSelector {
public:
    int epfd;

    std::unordered_map<int, uint32_t> fds;

    EpollSelector();

    void modify(int fd, uint32_t new_events);

    void add(int fd, uint32_t events = EPOLLIN);
    void remove(int fd, uint32_t events = EPOLLIN);

    void wait(size_t &nfds, epoll_event *events) const;
};

struct QueueItem {
    int fd = 0;
    char buffer[BUFF_SIZE]{};

    size_t sent = 0;
    size_t length = 0;

    bool skip_fd = false;

    void *extended = nullptr;
    void (*on_progress)(QueueItem *) = nullptr;
};

class EventLoop {
public:
    EpollSelector selector;
    bool running = true;

    std::vector<Future*> futures;
    std::deque<QueueItem*> packets_queue;

    Future &create_future(int fd, void (*_callback)(EventLoop *fut, Future *), uint32_t events = EPOLLIN);
    Future &insert_to_top(int fd, void (*_callback)(EventLoop *fut, Future *), uint32_t events = EPOLLIN);

    QueueItem &push_packet(void *buffer, int length, int fd, size_t pos = 0, bool skip_fd = true);
    bool in_queue(int fd, uint32_t events, size_t &epos);

    bool in_futures(epoll_event &ev, std::vector<Future*> &futs);

    void enable_if_can(int fd);

    void remove_by_fd(int fd, uint32_t events);
    void finish(Future *fut);

    void run();
};

#endif //OPENGROK_LOOP_HPP
