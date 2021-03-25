//
// Created by kvxmmu on 8/24/20.
//

#ifndef THINPAK_SELECTOR_HPP
#define THINPAK_SELECTOR_HPP


#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>

#include <unordered_map>

#ifndef MAX_EPOLL_SIZE
# define MAX_EPOLL_SIZE 4096
#endif


void set_nonblock(int fd);
void set_reuse(int fd);


class EpollSelector {
public:
    std::unordered_map<int, uint32_t> fds;
    int epfd;

    epoll_event events_buffer[MAX_EPOLL_SIZE]{};

    EpollSelector() : epfd(epoll_create(MAX_EPOLL_SIZE)) {

    }

    void add(int fd, uint32_t events = EPOLLIN);
    void modify(int fd, uint32_t events);
    void remove(int fd, uint32_t events,
            bool just_remove = false);

    bool has_flag(int fd, uint32_t flag);

    size_t wait(int timeout = -1);
};


#endif //THINPAK_SELECTOR_HPP
