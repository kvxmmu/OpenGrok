//
// Created by kvxmmu on 5/3/20.
//

#ifndef OPENGROK2_LOOP_HPP
#define OPENGROK2_LOOP_HPP

#include <iostream>
#include <unordered_set>
#include <vector>

#include <opengrok/packets_queue.hpp>

#include <sys/epoll.h>
#include <netinet/in.h>

#define TIMEOUT 5000
#define JUST_REMOVE 0xffff


class EventLoop;


class EpollSelector {
public:
    int epfd;

    int epoll_timeout = TIMEOUT;

    std::unordered_map<int, uint32_t> fds;

    EpollSelector();

    void add(int fd, uint32_t events = EPOLLIN);
    void modify(int fd, uint32_t events = EPOLLIN);
    void remove(int fd, uint32_t events = EPOLLIN);

    void remove_from_epoll(int fd, epoll_event &ev) const;

    void wait(size_t &nfds, epoll_event *events) const;
};


class AbstractProtocol {
public:
    EventLoop *loop = nullptr;
    bool is_server = true;

    int sockfd{};
    sockaddr_in addr{};

    unsigned short port;
    in_addr_t in_address;

    explicit AbstractProtocol(unsigned short _port, in_addr_t _in_address = INADDR_ANY);

    virtual void init(EventLoop *_loop) = 0;

    virtual void on_data_received(int fd) = 0;

    virtual void on_connect() = 0;

    virtual void on_disconnect(int fd) = 0;

    virtual void bind();

    virtual void dispose(){}
};

class EventLoop { // only for this project
public:
    bool running = false;

    EpollSelector main_selector;

    PacketsQueue packets_queue;

    AbstractProtocol *protocol = nullptr;

    std::unordered_map<int, AbstractProtocol*> protocols;
    std::unordered_map<int, int> proto_clients;

    std::unordered_set<int> clients;

    int create_client_to = -1;

    int sockfd = 0;

    explicit EventLoop(AbstractProtocol *proto);

    void perform_queue_tasks(epoll_event &ev);
    void add_client(int client_fd);
    void new_server(AbstractProtocol *proto);
    void close_all_connections_from_server(int server_fd,
            void (*callback)(int, void *) = nullptr, void *arg = nullptr);

    void push_packet(int fd, char *pkt, int size, void (*on_progress)(QueueItem *) = nullptr,
            void *data = nullptr);

    void run();
};




#endif //OPENGROK2_LOOP_HPP
