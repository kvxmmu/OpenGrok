//
// Created by nero on 05.07.2021.
//

#include "hincl.hpp"

void LLHttp::StreamingParser::on_header_field(const char *h_name, size_t h_length) {
    _cur_name = std::move(std::string(h_name, h_length));
}

void LLHttp::StreamingParser::on_header_value(const char *h_value, size_t h_length) {
    headers[_cur_name] = std::move(std::string(h_value, h_length));

    _cur_name.clear();
}

void LLHttp::StreamingParser::feed(const char *buffer, size_t length) {
    auto err = llhttp_execute(&llhttp, buffer, length);

    if (err != HPE_OK) {
        parser_callback->on_parse_error(*this, &llhttp);
    }
}

void LLHttp::StreamingParser::on_message_complete() {
    parser_callback->on_complete(*this, &llhttp);
    headers.clear();
}

void LLHttp::StreamingParser::feed(const std::string &str) {
    this->feed(str.data(), str.size());
}
