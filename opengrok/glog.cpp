//
// Created by kvxmmu on 3/25/21.
//

#include "glog.hpp"

void GLog::Logger::add_streamer(ILogStreamer *streamer) {
    this->streamers.emplace_back(streamer);
}

void GLog::Logger::add_stdout_streamer() {
    auto stdout_streamer = std::make_shared<StdoutStreamer>();

    this->streamers.push_back(stdout_streamer);
}

void GLog::Logger::add_file_streamer(const char *filename) {
    auto file_streamer = std::make_shared<FileStreamer>(filename);

    this->streamers.push_back(file_streamer);
}

void GLog::Logger::broadcast(const std::string &str) {
    for (auto &streamer : this->streamers) {
        streamer->write(this->app_name, str);
    }
}

void GLog::Logger::broadcast_endl() {
    for (auto &streamer : this->streamers) {
        streamer->write(GLog::endl);
    }
}




