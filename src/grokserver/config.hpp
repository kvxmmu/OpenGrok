//
// Created by nero on 12.06.2021.
//

#ifndef GROKPP_CONFIG_HPP
#define GROKPP_CONFIG_HPP

#include <string>
#include <cinttypes>

#define MAX_DECOMPRESSED_SIZE 16777216 // 16mb

class Config {
public:
    uint16_t http_port;

    uint16_t port;
    uint32_t v4_host;
    std::string server_name;

    uint32_t max_decompressed_size = MAX_DECOMPRESSED_SIZE;

    Config(uint16_t _port, uint32_t _host,
           std::string _server_name, uint16_t _http_port) : port(_port), v4_host(_host), server_name(std::move(_server_name)),
                                                            http_port(_http_port) {

    }
};

#endif //GROKPP_CONFIG_HPP
