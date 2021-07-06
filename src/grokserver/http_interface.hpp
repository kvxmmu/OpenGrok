//
// Created by nero on 06.07.2021.
//

#ifndef GROKPP_HTTP_INTERFACE_HPP
#define GROKPP_HTTP_INTERFACE_HPP


#include <loop/loop.hpp>
#include <hincl/hincl.hpp>

#include <memory>
#include <unordered_map>


const static char *index_page =  "<html lang=\"en\">\n"
                                 "<head>\n"
                                 "    <meta charset=\"UTF-8\">\n"
                                 "    <meta name=\"viewport\"\n"
                                 "          content=\"width=device-width, user-scalable=no, initial-scale=1.0, maximum-scale=1.0, minimum-scale=1.0\">\n"
                                 "    <meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">\n"
                                 "    <title>FreeGrok - Index page</title>\n"
                                 "</head>\n"
                                 "<body>\n"
                                 "    <style>\n"
                                 "    .greentext {\n"
                                 "        color: lightgreen;\n"
                                 "    }\n"
                                 "\n"
                                 "    .textcenter {\n"
                                 "        text-align: center;\n"
                                 "    }\n"
                                 "\n"
                                 "    .wipwarning {\n"
                                 "        color: darkred;\n"
                                 "    }\n"
                                 "\n"
                                 "    body {\n"
                                 "        background-color: #FAFAFA;\n"
                                 "    }\n"
                                 "\n"
                                 "    .mainblock {\n"
                                 "        margin: 2em;\n"
                                 "    }\n"
                                 "    </style>\n"
                                 "\n"
                                 "    <h1 class=\"textcenter\"><span class=\"greentext\">Free</span>Grok <span class=\"wipwarning\">[WIP]</span></h1>\n"
                                 "\n"
                                 "    <div class=\"mainblock\">\n"
                                 "        <p><strong>OpenSource</strong> dedicated ngrok analog.\n"
                                 "            <strong>FreeGrok</strong> is designed to be faster than any other project like it.</p>\n"
                                 "        <h1 id=\"features\">Features</h1>\n"
                                 "        <ul>\n"
                                 "            <li>ZSTD packets compression</li>\n"
                                 "            <li>Client/server windows compatibility</li>\n"
                                 "        </ul>\n"
                                 "        <h1 id=\"todo\">TODO</h1>\n"
                                 "        <ul>\n"
                                 "            <li><p>Timeout handling</p>\n"
                                 "                <p> For example, when a client sends an incomplete packet, FreeGrok will\n"
                                 "                    wait for it forever, ofc if packet is special crafted to fill your RAM with trash,\n"
                                 "                    use FreeGrok only when you sure that this will not happen. This is not dangerous, but\n"
                                 "                    I&#39;ll try to fix it, maybe tomorrow.</p>\n"
                                 "            </li>\n"
                                 "            <li>Whitelists/blacklists</li>\n"
                                 "            <li>Database based blacklists/whitelists</li>\n"
                                 "            <li>p2p mode</li>\n"
                                 "        </ul>\n"
                                 "        <h1 id=\"dependencies\">Dependencies</h1>\n"
                                 "        <ul>\n"
                                 "            <li>Compiler that supports C++17</li>\n"
                                 "            <li>CMake</li>\n"
                                 "            <li>Epoll or IOCP</li>\n"
                                 "        </ul>\n"
                                 "        <p>WARNING: precompiled windows dll library is only available on x64 arch, otherwise crosscompile libzstd by yourself</p>\n"
                                 "        <h1 id=\"question-answer\">question/answer</h1>\n"
                                 "        <ul>\n"
                                 "            <li><p>How can I contact with the developer?</p>\n"
                                 "                <p>  <a href=\"https://t.me/kvxmmu\">here</a> my native language is <strong>Russian</strong>, but you can contact with\n"
                                 "                    me using <strong>English</strong></p>\n"
                                 "            </li>\n"
                                 "            <li><p>Why should i use <strong>FreeGrok</strong> instead of for example <strong>hamachi</strong> or <strong>Radmin</strong>?</p>\n"
                                 "                <p>  There is no particular reason to use <strong>FreeGrok</strong> instead of any other program that can\n"
                                 "                    open your port through NAT, but if you have virtual or dedicated server\n"
                                 "                    with a <strong>dedicated IPv4</strong> you can use the <strong>FreeGrok</strong> because it is faster or\n"
                                 "                    you have lower ping to the server. Also <strong>FreeGrok</strong> has compression!</p>\n"
                                 "            </li>\n"
                                 "        </ul>\n"
                                 "        <h1 id=\"config-format\">Config format</h1>\n"
                                 "        <p><strong>FreeGrok</strong> uses ini format for configs</p>\n"
                                 "        <p>By default FreeGrok takes the config file from the /etc directory, but you can change it by editing\n"
                                 "            constant <strong>CONFIG_FILE_LOCATION</strong> in src/freegrok/defaults.hpp</p>\n"
                                 "        <h4 id=\"example-config\">Example config</h4>\n"
                                 "        <p><a href=\"freegrok.ini.example\">here</a></p>\n"
                                 "        <h1 id=\"client\">Client</h1>\n"
                                 "        <p>Client can be found <a href=\"https://github.com/kvxmmu/freegrok_client\">here</a></p>\n"
                                 "\n"
                                 "    </div>\n"
                                 "</body>\n"
                                 "</html>";
const static auto index_page_length = strlen(index_page);


class HTTPPeer {
public:
    LLHttp::StreamingParser parser;
    sockaddr_in peer{};

    HTTPPeer() = default;

    explicit
    HTTPPeer(LLHttp::IParserCallback *_callback, sock_t fd, llhttp_type_t type = HTTP_REQUEST) : parser(_callback, type) {
        parser.u64 = static_cast<uint64_t>(fd);
    }
};


class HttpInterface : public IObserver, public LLHttp::IParserCallback {
public:
    std::unordered_map<sock_t, std::shared_ptr<HTTPPeer>> parsers;
    uint16_t port;

    explicit
    HttpInterface(uint16_t _port) : IObserver(tcp_create(), true),
                                    port(_port) {

    }

    void post_init() override;

    sock_t on_connect() override;
    void on_disconnect(sock_t sock) override;
    void on_received(state_t state, ReadItem &item) override;

    /// LLHttp::IParserCallback

    void on_body(LLHttp::StreamingParser &parser, const char *ptr, size_t length) override;
    void on_complete(LLHttp::StreamingParser &parser, llhttp_t *llhttp) override;
    void on_parse_error(LLHttp::StreamingParser &parser, llhttp_t *llhttp) override;
};



#endif //GROKPP_HTTP_INTERFACE_HPP
