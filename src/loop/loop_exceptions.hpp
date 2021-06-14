//
// Created by nero on 12.06.2021.
//

#ifndef GROKPP_LOOP_EXCEPTIONS_HPP
#define GROKPP_LOOP_EXCEPTIONS_HPP

#include <stdexcept>

class EmptyQueue : std::runtime_error {
public:
    enum Type {
        READ, WRITE
    };

    Type type;

    explicit
    EmptyQueue(const char *s, Type _type) : std::runtime_error(s), type(_type) {

    }

    static void raise(Type type) {
        char strbuf[640]{0};
        strcat(strbuf, type == READ ? "Read " : "Write ");
        strcat(strbuf, "queue is empty");

        throw EmptyQueue(strbuf, type);
    }
};

class NoObservers : public std::runtime_error {
public:
    explicit
    NoObservers(const char *s) : std::runtime_error(s) {

    }
};

#endif //GROKPP_LOOP_EXCEPTIONS_HPP
