//
// Created by nero on 12.06.2021.
//

#ifndef GROKPP_CONFIG_HPP
#define GROKPP_CONFIG_HPP

#include <string>
#include <cinttypes>

class Config {
public:
    uint16_t port;
    uint32_t v4_host;
    std::string server_name;

    Config(uint16_t _port, uint32_t _host,
           std::string _server_name) : port(_port), v4_host(_host), server_name(std::move(_server_name)) {

    }
};

#endif //GROKPP_CONFIG_HPP
