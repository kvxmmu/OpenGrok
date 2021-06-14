//
// Created by nero on 12.06.2021.
//

#include "proxy_client.hpp"

void GrokProxyClient::post_init() {
    std::cout << "[FreeGrok:ProxyClient] Connecting to the local server..." << std::endl;

    tcp_connect(socket, connect_host, connect_port);
}

void GrokProxyClient::on_received(state_t state, ReadItem &item) {
    char buf[GROK_PROXY_RECV_BUF_SIZE];
    auto received = tcp_recv(socket, buf, GROK_PROXY_RECV_BUF_SIZE);

    client->redirect_packet(self_id, buf, received);
}

sock_t GrokProxyClient::on_connect() {
    std::cout << "[FreeGrok:ProxyClient] Successfully connected" << std::endl;
    loop->capture(socket);

    return socket;
}

void GrokProxyClient::on_disconnect(sock_t sock) {
    std::cout << "[FreeGrok:ProxyClient] Local client is disconnected" << std::endl;
    if (!shut) client->send_disconnected(self_id);
}

void GrokProxyClient::redirect(char *buffer, size_t length) {
    loop->send(socket, buffer, length);
}
