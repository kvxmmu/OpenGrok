//
// Created by kvxmmu on 5/3/20.
//

#include "loop.hpp"


// EpollSelector class

EpollSelector::EpollSelector() : epfd(epoll_create(MAX_EPOLL_EVENTS)) {

}

void EpollSelector::modify(int fd, uint32_t events) {
    epoll_event ev{};
    ev.data.fd = fd;
    ev.events = events;

    if (epoll_ctl(this->epfd, EPOLL_CTL_MOD, fd, &ev) != 0) {
        perror("epoll_ctl() mod");
    } else {
        this->fds[fd] = events;
    }
}

void EpollSelector::add(int fd, uint32_t events) {
    if (this->fds.find(fd) != this->fds.end()) {
        this->modify(fd, this->fds.at(fd) | events);
        return;
    }

    epoll_event ev{};
    ev.data.fd = fd;
    ev.events = events;

    if (epoll_ctl(this->epfd, EPOLL_CTL_ADD, fd, &ev) != 0) {
        perror("epoll_ctl() add");
    } else {
        this->fds.emplace(std::make_pair(fd, events));
    }
}

void EpollSelector::remove(int fd, uint32_t events) {
    epoll_event ev{};
    ev.data.fd = fd;
    ev.events = events;

    if (this->fds.find(fd) != this->fds.end()) {
        uint32_t fd_events = this->fds.at(fd);

        if (fd_events == events || events == JUST_REMOVE) {
            ev.events = fd_events;
            this->remove_from_epoll(fd, ev);
            this->fds.erase(fd);
        } else {
            this->modify(fd, fd_events & ~events);
        }

    } else {
        std::cout << NAME << SPACE << "remove(): Unknown FD#" << fd << std::endl;
    }


}

void EpollSelector::wait(size_t &nfds, epoll_event *events) const {
    nfds = epoll_wait(this->epfd, events, MAX_EPOLL_EVENTS, this->epoll_timeout);
}

void EpollSelector::remove_from_epoll(int fd, epoll_event &ev) const {
    if (epoll_ctl(this->epfd, EPOLL_CTL_DEL, fd, &ev) != 0) {
        perror("epoll_ctl() remove");
    }
}


// AbstractServer class

AbstractProtocol::AbstractProtocol(unsigned short _port, in_addr_t _in_address) : port(_port), in_address(_in_address){
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);

    this->addr = create_addr(_port, _in_address);
}

void AbstractProtocol::bind() {
    set_reuse(this->sockfd);

    ::bind(this->sockfd, reinterpret_cast<sockaddr *>(&this->addr), sizeof(this->addr));
    listen(this->sockfd, MAX_EPOLL_EVENTS);
}

// EventLoop

EventLoop::EventLoop(AbstractProtocol *proto) : protocol(proto) {

}

void EventLoop::run() {
    this->running = true;

    this->protocol->init(this);
    this->main_selector.add(this->sockfd, EPOLLIN);

    std::unordered_set<int> skip;
    char test_buff;

    size_t nfds;
    epoll_event events[MAX_EPOLL_EVENTS];

    int bytes;

    while (this->running) {
        skip.clear();
        skip.rehash(0);

        this->main_selector.wait(nfds, events);

        for (size_t pos = 0; pos < nfds; pos++) {
            epoll_event &ev = events[pos];

            if (skip.find(ev.data.fd) != skip.end())
                continue;

            if (ev.events & EPOLLOUT) {
                this->perform_queue_tasks(ev);
            } else if (ev.data.fd == this->protocol->sockfd) {
                if (this->protocol->is_server)
                    this->protocol->on_connect();
                else {
                    char pedik;
                    bytes = recv(ev.data.fd, &pedik, sizeof(char), MSG_PEEK);

                    if (bytes == 0)
                        this->protocol->on_disconnect(ev.data.fd);
                    else
                        this->protocol->on_data_received(ev.data.fd);
                }
            } else if (this->clients.find(ev.data.fd) != this->clients.end()) {
                bytes = recv(ev.data.fd, &test_buff, sizeof(char), MSG_PEEK);

                if (bytes == 0) {
                    this->protocol->on_disconnect(ev.data.fd);
                    continue;
                }

                this->protocol->on_data_received(ev.data.fd);
            } else if (this->proto_clients.find(ev.data.fd) != this->proto_clients.end()) {
                int server_fd = this->proto_clients.at(ev.data.fd);
                AbstractProtocol *server_protocol = this->protocols.at(server_fd);

                bytes = recv(ev.data.fd, &test_buff, sizeof(char), MSG_PEEK);

                if (bytes == 0) {
                    server_protocol->on_disconnect(ev.data.fd);
                    continue;
                }
                server_protocol->on_data_received(ev.data.fd);
            } else if (this->protocols.find(ev.data.fd) != this->protocols.end()) {
                AbstractProtocol *server_protocol = this->protocols.at(ev.data.fd);

                this->create_client_to = server_protocol->sockfd;
                server_protocol->on_connect();
                this->create_client_to = -1;
            }
        }

    }
}

void EventLoop::perform_queue_tasks(epoll_event &ev) {
    bool end;

    if (!this->packets_queue.check(ev.data.fd, end)) {
        std::cout << NAME << SPACE << "Unknwown FD#" << ev.data.fd << " in QueueSelector" << std::endl;
        return;
    }
    if (end)
        this->main_selector.remove(ev.data.fd, EPOLLOUT);
}

void EventLoop::add_client(int client_fd) {
    this->main_selector.add(client_fd, EPOLLIN);

    if (this->create_client_to == -1)
        this->clients.emplace(client_fd);
    else
        this->proto_clients.emplace(std::make_pair(client_fd, this->create_client_to));
}

void EventLoop::new_server(AbstractProtocol *proto) {
    proto->init(this);
    this->protocols.emplace(std::make_pair(proto->sockfd, proto));

    this->main_selector.add(proto->sockfd);
}

void EventLoop::push_packet(int fd, char *pkt, int size, void (*on_progress)(QueueItem *), void *data) {
    this->packets_queue.push(fd, pkt, size, on_progress, data);
    this->main_selector.add(fd, EPOLLOUT);
}

void EventLoop::close_all_connections_from_server(int server_fd, void (*callback)(int, void *), void *arg) {
    std::vector<int> keys;

    for (auto &pair : this->proto_clients) {
        if (pair.second == server_fd) {
            if (callback != nullptr)
                callback(pair.first, arg);
            close(pair.first);
            keys.push_back(pair.first);
        }
    }

    for (int &key : keys) {
        this->proto_clients.erase(key);
    }

    keys.clear();
    keys.shrink_to_fit();
}
