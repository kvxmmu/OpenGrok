//
// Created by kvxmmu on 3/25/21.
//

#ifndef GROKLOOP_ABSTRACT_HPP
#define GROKLOOP_ABSTRACT_HPP

class GrokLoop;

class AbstractObserver {
public:
    bool is_server;

    GrokLoop *loop = nullptr;

    explicit AbstractObserver(bool _is_server) : is_server(_is_server) {

    }

    virtual void on_connect() = 0;
    virtual void on_disconnect(int fd) = 0;

    virtual void init(GrokLoop *_loop) {
        this->loop = _loop;
    }
};

#endif //GROKLOOP_ABSTRACT_HPP
