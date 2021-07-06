//
// Created by nero on 06.07.2021.
//

#include "http_interface.hpp"

void HttpInterface::post_init() {
    tcp_set_reuse(socket);
    tcp_bind(socket, INADDR_ANY, port);
    tcp_listen(socket);

    std::cout << "[FreeGrok:HttpInterface] Started HTTP Server on port :" << port << std::endl;
}

sock_t HttpInterface::on_connect() {
    sockaddr_in caddr{};
    auto client_fd = tcp_accept(socket, caddr);
    std::cout << "[FreeGrok:HttpInterface] Connected to the HTTP interface client " << inet_ntoa(caddr.sin_addr) << std::endl;
    loop->capture(client_fd);

    parsers[client_fd] = std::make_shared<HTTPPeer>(this, client_fd);

    return client_fd;
}

void HttpInterface::on_disconnect(sock_t sock) {
    if (parsers.find(sock) == parsers.end()) {
        std::cerr << "[FreeGrok:HttpInterface] Error: socket does not exist " << sock << std::endl;
        // should be never called

        return;
    }

    auto peer = parsers[sock]->peer;

    parsers.erase(sock);
    std::cout << "[FreeGrok:HttpInterface] Disconnected client " << inet_ntoa(peer.sin_addr) << std::endl;
}

void HttpInterface::on_received(state_t state, ReadItem &item) {
    if (parsers.find(item.fd) == parsers.end()) {
        std::cerr << "[FreeGrok:HttpInterface] Error: socket does not exist " << item.fd << std::endl;
        // should be never called

        return;
    }

    char k_buffer[4096]; // 4kb by default
    auto received = tcp_recv(item.fd, k_buffer, 4096);

    auto &peer = parsers[item.fd];
    auto &parser = peer->parser;

    parser.feed(k_buffer, received);
}

void HttpInterface::on_body(LLHttp::StreamingParser &parser, const char *ptr, size_t length) {
    //  а ты веселый
    //  TODO: add body handling
}

void HttpInterface::on_complete(LLHttp::StreamingParser &parser, llhttp_t *llhttp) {
    auto fd = static_cast<sock_t>(parser.u64);
    const static char *header_buff = "HTTP/1.1 200 OK\r\n"
                                     "Connection: keep-alive\r\n"
                                     "Server: Embedded FreeGrok\r\n";
    const static auto header_len = strlen(header_buff);
    const static std::string body_len = "Content-Length: "+std::to_string(index_page_length)+"\r\n\r\n";

    loop->send(fd, const_cast<char *>(header_buff), header_len);
    loop->send(fd, const_cast<char *>(body_len.c_str()), body_len.size());
    loop->send(fd, const_cast<char *>(index_page), index_page_length);

}

void HttpInterface::on_parse_error(LLHttp::StreamingParser &parser, llhttp_t *llhttp) {
    auto fd = static_cast<sock_t>(parser.u64);

    const static char *header_buff = "HTTP/1.1 400 Bad Request\r\n"
                                     "Connection: keep-alive\r\n"
                                     "Server: Embedded FreeGrok\r\n"
                                     "\r\nBad request";
    const static auto header_len = strlen(header_buff);


    loop->send(fd, const_cast<char *>(header_buff), header_len);
    loop->send(fd, const_cast<char *>(index_page), index_page_length);

}
