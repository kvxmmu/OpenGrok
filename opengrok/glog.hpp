//
// Created by kvxmmu on 3/25/21.
//

#ifndef GROKLOOP_GLOG_HPP
#define GROKLOOP_GLOG_HPP

#include <iostream>
#include <fstream>
#include <vector>

#include <memory>
#include <iostream>

#include "log_interface.hpp"
#include "log_errors.hpp"

namespace GLog {
    class FileStreamer : public ILogStreamer {
    public:
        std::ofstream stream;

        explicit FileStreamer(const char *filename) {
            this->stream.open(filename);

            if (!this->stream.is_open()) {
                throw Exceptions::FileOpeningError(filename);
            }
        }

        void write(const std::string &str) override {
            this->stream.write(str.c_str(), str.size());
        }

        void write(ILogEndl &__endl) override {
            ILogStreamer::write(__endl);

            this->stream.write("\n", 1);
        }

        void drain() override {
            this->stream.flush();
        }
    };

    class StdoutStreamer : public ILogStreamer {
    public:
        void write(ILogEndl &__endl) override {
            ILogStreamer::write(__endl);

            std::cout << std::endl;
        }

        void write(const std::string &str) override {
            std::cout << str;
        }

        void drain() override {
            std::cout.flush();
        }
    };

    class Logger {
    public:
        std::string app_name;
        std::vector<std::shared_ptr<ILogStreamer>> streamers;

        explicit Logger(const char *_app_name) : app_name(_app_name) {

        }

        void add_streamer(ILogStreamer *streamer);

        void add_stdout_streamer();
        void add_file_streamer(const char *filename);

        void broadcast(const std::string &str);
        void broadcast_endl();

        friend Logger &operator<<(GLog::Logger &logger, int integer) {
            logger.broadcast(std::to_string(integer));

            return logger;
        }

        friend Logger &operator<<(GLog::Logger &logger, float floating_point) {
            logger.broadcast(std::to_string(floating_point));

            return logger;
        }

        friend Logger &operator<<(GLog::Logger &logger, const std::string &str) {
            logger.broadcast(str);

            return logger;
        }

        friend Logger &operator<<(GLog::Logger &logger, ILogEndl &) {
            logger.broadcast_endl();

            return logger;
        }

        friend Logger &operator<<(GLog::Logger &logger, const char *view) {
            logger.broadcast(view);

            return logger;
        }
    };
}

#endif //GROKLOOP_GLOG_HPP
