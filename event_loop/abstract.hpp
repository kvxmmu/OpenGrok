//
// Created by kvxmmu on 3/25/21.
//

#ifndef GROKLOOP_ABSTRACT_HPP
#define GROKLOOP_ABSTRACT_HPP

class GrokLoop;

class AbstractObserver {
public:
    bool is_server;
    bool is_connected = false;

    int sockfd;

    GrokLoop *loop = nullptr;

    explicit AbstractObserver(bool _is_server, int _sockfd) : is_server(_is_server), sockfd(_sockfd) {

    }

    virtual int on_connect() = 0;
    virtual void on_disconnect(int fd) = 0;

    virtual void init(GrokLoop *_loop) {
        this->loop = _loop;
    }

    virtual void post_init() {}
};

#endif //GROKLOOP_ABSTRACT_HPP
