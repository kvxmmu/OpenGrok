//
// Created by kvxmmu on 3/27/21.
//

#ifndef CAMELEO_ABSTRACT_HPP
#define CAMELEO_ABSTRACT_HPP

#include <unordered_set>

namespace Cameleo {
    class EventLoop;

    class IObserver {
    public:
        bool is_server;
        bool is_connected;

        int sockfd;

        std::unordered_set<int> __clients;

        EventLoop *loop = nullptr;

        IObserver(int _sockfd, bool _is_server) : sockfd(_sockfd), is_connected(false), is_server(_is_server) {

        }

        bool __has_client(int fd) {
            return this->__clients.find(fd) != this->__clients.end();
        }

        void __add_client(int fd) {
            this->__clients.insert(fd);
        }

        virtual void init(EventLoop *_loop) {
            this->loop = _loop;
        }
        virtual void post_init() {};

        virtual int on_connect() = 0;
        virtual void post_connect(int fd) {}
        virtual void on_disconnect() = 0;

        virtual ~IObserver() = default;
    };
}

#endif //CAMELEO_ABSTRACT_HPP
