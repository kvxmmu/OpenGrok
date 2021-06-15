//
// Created by nero on 12.06.2021.
//

#include "server.hpp"

//// Message processor

void FreeGrok::process_message(uint8_t type, uint32_t length, FStreamer &streamer) {
    auto &reader = streamer.reader;
    auto fd = streamer.target;

    switch (type) {
        case GROK_PING: {
            streamer.send(GROK_PING, cfg.server_name.data(), cfg.server_name.size());

            break;
        }

        case GROK_CREATE_SERVER: {
            if (servers.find(fd) != servers.end()) {
                streamer.send_error(SERVER_ALREADY_CREATED);

                break;
            }

            auto proxy_server = new GrokProxy(this, fd);
            servers[fd] = proxy_server;
            loop->add_observer(proxy_server);

            if (proxy_server->listening_port == 0) {
                streamer.send_error(INTERNAL_SERVER_ERROR);
                loop->remove_observer(proxy_server);
                servers.erase(fd);

                break;
            }

            char ls_port_buf[sizeof(uint16_t)];
            int_to_bytes<uint16_t>(proxy_server->listening_port, ls_port_buf);

            streamer.send(GROK_CREATE_SERVER, ls_port_buf, sizeof(uint16_t));

            break;
        }

        case GROK_PACKET: {
            if (servers.find(fd) == servers.end()) {
                streamer.send_error(NO_SESSION);

                break;
            } else if (length <= sizeof(c_id_t)) {
                streamer.send_error(TOO_SHORT_BUFFER);

                break;
            }

            auto client_id = reader.read<c_id_t>();
            auto server = servers[fd];

            if (!server->has_client(client_id)) {
                streamer.send_error(NO_SUCH_CLIENT);

                break;
            }

            server->redirect_packet(client_id, reader.buffer+reader.offset,
                                    length - reader.offset);

            break;
        }

        case GROK_DISCONNECT: {
            if (servers.find(fd) == servers.end()) {
                streamer.send_error(NO_SESSION);

                break;
            } else if (length != sizeof(c_id_t)) {
                streamer.send_error(BUFFER_SIZE_INCORRECT);

                break;
            }

            auto client_id = reader.read<c_id_t>();
            auto server = servers[fd];

            if (!server->has_client(client_id)) {
                streamer.send_error(NO_SUCH_CLIENT);

                break;
            }

            server->disconnect_client(client_id);

            break;
        }

        case GROK_STAT: {
            streamer.send_error(NOT_IMPLEMENTED);

            break;
        }

        default: {
            std::cout << "[FreeGrok] Unexpected command sent " << (int)type << std::endl;
            streamer.send_error(NO_SUCH_COMMAND);

            break;
        }
    }
}

////

void FreeGrok::post_init() {
    /// Initializing server

    tcp_set_reuse(socket);
    tcp_bind(socket, cfg.v4_host,
             cfg.port);
    tcp_listen(socket);

    std::cout << "[FreeGrok] Listening on port " << cfg.port << std::endl;
}

sock_t FreeGrok::on_connect() {
    sockaddr_in addr{};
    auto client_fd = tcp_accept(socket, addr);

    std::cout << "[FreeGrok] Received connection from " << inet_ntoa(addr.sin_addr) << std::endl;
    this->go_new_message(client_fd);

    return client_fd;
}

void FreeGrok::on_received(state_t _state, ReadItem &item) {
    auto state = static_cast<PacketState>(_state);

    switch (state) {
        case TYPE_GOT: {
            uint8_t data = item.buffer[0];
            uint8_t length_chunk = item.buffer[1];
            uint8_t type = GROK_GET_TYPE(data);

            if (likely(GROK_IS_SHORT(data))) {
                if (length_chunk == 0u) {
                    FStreamer streamer(loop, item.fd,
                                       nullptr);

                    this->process_message(type, length_chunk, streamer);
                    this->go_new_message(item.fd);

                    break;
                }

                auto meta = (static_cast<uint64_t>(data) << 32u) | static_cast<uint32_t>(length_chunk);
                loop->recv(item.fd, length_chunk,
                           BUFFER_GOT, meta);
            } else {
                uint16_t meta = (static_cast<uint16_t>(data) << 8u) | length_chunk;

                loop->recv(item.fd, 3, HEADER_GOT,
                           static_cast<uint64_t>(meta));
            }

            break;
        }

        case HEADER_GOT: {
            auto meta = static_cast<uint16_t>(item.u64);
            auto length_chunk = static_cast<uint8_t>(meta & 0xffu);
            auto data = static_cast<uint8_t>(meta >> 8u);
            auto type = GROK_GET_TYPE(data);
            unsigned char length_buf[sizeof(uint32_t)];
            length_buf[0] = length_chunk;
            memcpy(reinterpret_cast<char *>(length_buf + 1),
                   item.buffer, sizeof(uint32_t) - sizeof(uint8_t));

            auto length = bytes_to_int<uint32_t>(reinterpret_cast<char *>(length_buf));

            if (unlikely(length == 0)) {
                FStreamer streamer(loop, item.fd,
                                   item.buffer+3u);

                this->process_message(type, length, streamer);
                this->go_new_message(item.fd);

                break;
            }

            uint64_t merged = (static_cast<uint64_t>(data) << 32u) | length;
            loop->recv(item.fd, length,
                       BUFFER_GOT, merged);

            break;
        }

        case BUFFER_GOT: {
            auto merged = item.u64;
            auto data = static_cast<uint8_t>(merged >> 32u);
            auto type = GROK_GET_TYPE(data);
            auto length = static_cast<uint32_t>(merged & 0xffffffffu);

            char *used_buffer = item.buffer;
            bool need_free = false;

            if (likely(GROK_IS_COMPRESSED(data))) {
                auto buff_size = ZSTD_getFrameContentSize(used_buffer, length);

                if (buff_size == ZSTD_CONTENTSIZE_UNKNOWN ||
                    buff_size == ZSTD_CONTENTSIZE_ERROR || buff_size == 0) {
                    FStreamer streamer(loop, item.fd,
                                       nullptr);
                    streamer.send_error(DECOMPRESSION_ERROR);
                    this->go_new_message(item.fd);

                    break;
                } else if (buff_size > MAX_DECOMPRESSED_SIZE) {
                    FStreamer streamer(loop, item.fd, nullptr);
                    streamer.send_error(TOO_LONG_BUFFER); // too long decompression buffer
                    this->go_new_message(item.fd);

                    break;
                }

                char *dec_buf = new char[buff_size];
                auto response = ZSTD_decompress(dec_buf, buff_size,
                                                used_buffer, length);

                if (ZSTD_isError(response)) {
                    delete[] dec_buf;

                    FStreamer streamer(loop, item.fd, nullptr);
                    streamer.send_error(DECOMPRESSION_ERROR);
                    this->go_new_message(item.fd);

                    break;
                }

                used_buffer = dec_buf;
                length = static_cast<uint32_t>(buff_size);
                need_free = true;
            }

            FStreamer streamer(loop, item.fd,
                               used_buffer);

            this->process_message(type, length, streamer);
            this->go_new_message(item.fd);

            if (need_free) {
                delete[] used_buffer;
            }

            break;
        }
    }
}

void FreeGrok::on_disconnect(sock_t sock) {
    sockaddr_in addr{};
    addr.sin_addr.s_addr = 0xffffffffu;
    tcp_get_peer_name(sock, addr);

    std::cout << "[FreeGrok] Disconnected client " << inet_ntoa(addr.sin_addr) << std::endl;

    if (unlikely(servers.find(sock) != servers.end())) {
        auto serv = servers[sock];
        serv->shutdown(); // free ids

        loop->remove_observer(serv);
        delete serv;
        servers.erase(sock);
    }
}

void FreeGrok::go_new_message(sock_t src) {
    loop->recv(src, sizeof(uint16_t),
               PacketState::TYPE_GOT);
}

//// IMainServer

void FreeGrok::send_connected(sock_t initiator, c_id_t generated_id) {
    char id_buf[sizeof(c_id_t)];
    int_to_bytes(generated_id, id_buf);

    FStreamer streamer(loop, initiator, nullptr);
    streamer.send(GROK_CONNECTED, id_buf, sizeof(c_id_t));
}

void FreeGrok::redirect_packet(sock_t initiator, c_id_t sender, char *buffer, size_t length) {
    char *buf = new char[sizeof(c_id_t)+length];
    int_to_bytes(sender, buf);
    memcpy(buf+sizeof(c_id_t), buffer, length);

    FStreamer streamer(loop, initiator, nullptr);
    streamer.send(GROK_PACKET, buf, sizeof(c_id_t)+length);

    delete[] buf;
}

void FreeGrok::send_disconnected(sock_t initiator, c_id_t client_id) {
    char client_buf[sizeof(c_id_t)];
    int_to_bytes(client_id, client_buf);

    FStreamer streamer(loop, initiator, nullptr);
    streamer.send(GROK_DISCONNECT, client_buf, sizeof(c_id_t));
}

