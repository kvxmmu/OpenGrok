//
// Created by kvxmmu on 3/25/21.
//

#ifndef GROKLOOP_EVENT_LOOP_ERRORS_HPP
#define GROKLOOP_EVENT_LOOP_ERRORS_HPP

#include <stdexcept>

namespace Exceptions {
    class UnknownEpollEvent : public std::runtime_error {
    public:
        uint32_t epoll_ev;

        explicit UnknownEpollEvent(uint32_t event) : epoll_ev(event), std::runtime_error("Unknown epoll event") {

        }
    };

    class NoObservers : public std::runtime_error {
    public:
        NoObservers() : std::runtime_error("No observers in the event loop") {

        }
    };

    class NoFutures : public std::runtime_error {
    public:
        int fd;

        NoFutures(int for_fd) : fd(for_fd), std::runtime_error("No more pending futures in the loop") {

        }
    };
}

#endif //GROKLOOP_EVENT_LOOP_ERRORS_HPP
