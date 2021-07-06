//
// Created by nero on 05.07.2021.
//

#ifndef HTTP_INCL_HINCL_HPP
#define HTTP_INCL_HINCL_HPP

#include "llhttp.h"
#include <unordered_map>
#include <string>

#include <iostream>


namespace LLHttp {
    class StreamingParser;

    class IParserCallback {
    public:
        virtual void on_body(StreamingParser &parser, const char *ptr, size_t length) = 0;
        virtual void on_parse_error(StreamingParser &parser, llhttp_t *llhttp) = 0;
        virtual void on_complete(StreamingParser &parser, llhttp_t *llhttp) = 0;
    };

    class StreamingParser {
    private:
        std::string _cur_name;

    protected:
        void on_header_field(const char *h_name, size_t h_length);
        void on_header_value(const char *h_value, size_t h_length);

        void on_message_complete();

    public:
        llhttp_t llhttp{};
        llhttp_settings_t settings{};
        IParserCallback *parser_callback;

        std::unordered_map<std::string, std::string> headers;

        uint64_t u64{0};

        StreamingParser() = default;

        explicit
        StreamingParser(IParserCallback *_callback, llhttp_type_t type = HTTP_BOTH) : parser_callback(_callback) {
            llhttp_settings_init(&settings);

            settings.on_header_field = [](llhttp_t *_llhttp, const char *at, size_t length) {
                reinterpret_cast<StreamingParser *>(_llhttp->data)->on_header_field(at, length);

                return 0;
            };
            settings.on_header_value = [](llhttp_t *_llhttp, const char *at, size_t length) {
                reinterpret_cast<StreamingParser *>(_llhttp->data)->on_header_value(at, length);

                return 0;
            };
            settings.on_message_complete = [](llhttp_t *parser) {
                reinterpret_cast<StreamingParser *>(parser->data)->on_message_complete();

                return 0;
            };
            settings.on_body = [](llhttp_t *_llhttp, const char *at, size_t length) {
                auto t = reinterpret_cast<StreamingParser *>(_llhttp->data);
                t->parser_callback->on_body(*t, at, length);

                return 0;
            };

            llhttp_init(&llhttp, HTTP_BOTH, &settings);
            llhttp.data = this;
        }

        void feed(const char *buffer, size_t length);
        void feed(const std::string &str);
    };
}



#endif //HTTP_INCL_HINCL_HPP
