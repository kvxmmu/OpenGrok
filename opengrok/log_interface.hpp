//
// Created by kvxmmu on 3/25/21.
//

#ifndef GROKLOOP_LOG_INTERFACE_HPP
#define GROKLOOP_LOG_INTERFACE_HPP

#include <string>

class ILogEndl {
public:
};

class ILogStreamer {
public:
    bool endl_written = true;

    virtual void write(const std::string &str) = 0;

    virtual void write(const std::string &app_name, const std::string &str) {
        if (this->endl_written) {
            this->write('['+app_name+"] ");

            this->endl_written = false;
        }

        this->write(str);
    }

    virtual void write(ILogEndl &endl) {
        this->endl_written = true;
    }

    virtual void drain() = 0;

    virtual ~ILogStreamer() = default;
};



namespace GLog { static ILogEndl endl; }

#endif //GROKLOOP_LOG_INTERFACE_HPP
