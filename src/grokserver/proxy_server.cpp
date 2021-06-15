//
// Created by nero on 12.06.2021.
//

#include "proxy_server.hpp"

c_id_t GrokProxy::link_client(sock_t sock) {
    auto id = server->id_pool.get_id();
    sock_to_id[sock] = id;
    id_to_sock[id] = sock;

    return id;
}

void GrokProxy::unlink_client(c_id_t id, sock_t sock, bool erase) {
    server->id_pool.remove_id(id);

    if (erase) {
        sock_to_id.erase(sock);
        id_to_sock.erase(id);
    }
}

void GrokProxy::post_init() {
    tcp_set_reuse(socket);
    tcp_bind(socket, INADDR_ANY, PROXY_PORT);
    tcp_listen(socket);

    listening_port = tcp_get_listening_port(socket);

    std::cout << "[FreeGrok:ProxyServer] Listening on port " << listening_port << std::endl;
}

sock_t GrokProxy::on_connect() {
    sockaddr_in addr{};
    auto client_fd = tcp_accept(socket, addr);

    std::cout << "[FreeGrok:ProxyServer] " << inet_ntoa(addr.sin_addr) << " connected to the "
              << inet_ntoa(initiator_peer.sin_addr) << " initiator" << std::endl;
    loop->capture(client_fd);
    server->send_connected(initiator, this->link_client(client_fd));

    return client_fd;
}

void GrokProxy::on_received(state_t state, ReadItem &item) {
    char redir_buf[REDIR_BUF_SIZE];
    auto received = tcp_recv(item.fd, redir_buf, REDIR_BUF_SIZE);

    server->redirect_packet(initiator, sock_to_id.at(item.fd),
                            redir_buf, received);
}

void GrokProxy::on_disconnect(sock_t sock) {
    auto id = sock_to_id.at(sock);
    this->unlink_client(id, sock);

    server->send_disconnected(initiator, id);
}

void GrokProxy::shutdown() {
    for (auto &s_pair : sock_to_id) {
        this->unlink_client(s_pair.second,
                            s_pair.first, false);
    }

    sock_to_id.clear();
    id_to_sock.clear();
}

void GrokProxy::redirect_packet(c_id_t client_id, char *buffer, size_t length) {
    auto client_fd = id_to_sock.at(client_id);

    loop->send(client_fd, buffer, length);
}

void GrokProxy::disconnect_client(c_id_t client_id) {
    auto client_fd = id_to_sock.at(client_id);
    this->unlink_client(client_id, client_fd);

    loop->force_disconnect(client_fd);
}

bool GrokProxy::has_client(c_id_t id) {
    return id_to_sock.find(id) != id_to_sock.end();
}
