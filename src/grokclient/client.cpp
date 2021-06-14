//
// Created by nero on 12.06.2021.
//

#include "client.hpp"


void GrokClient::process_message(uint8_t type, uint32_t length,
                                 FStreamer &streamer) {
    auto &reader = streamer.reader;

    switch (type) {
        case GROK_PING: {
            std::cout << "[FreeGrok] Ping: " << std::string(reader.buffer, length) << std::endl;
            streamer.send_header(GROK_CREATE_SERVER, 0);

            break;
        }

        case GROK_ERROR: {
            auto error_type = reader.read<uint8_t>();

            std::cout << "[FreeGrok] Eror type: 0x" << std::hex << static_cast<uint32_t>(error_type) << std::endl;

            break;
        }

        case GROK_CREATE_SERVER: {
            listening_port = reader.read<uint16_t>();

            std::cout << "[FreeGrok] Listening on port " << listening_port << std::endl;

            break;
        }

        case GROK_CONNECTED: {
            auto client_id = reader.read<c_id_t>();
            auto proxy = new GrokProxyClient(this, client_id,
                                             host, port);
            loop->add_observer(proxy);
            proxies[client_id] = proxy;

            break;
        }

        case GROK_PACKET: {
            auto client_id = reader.read<c_id_t>();
            auto proxy = proxies.at(client_id);

            proxy->redirect(reader.buffer+reader.offset, length - reader.offset);

            break;
        }

        case GROK_DISCONNECT: {
            auto client_id = reader.read<c_id_t>();
            auto proxy = proxies.at(client_id);

            proxy->shutdown();
            loop->remove_observer(proxy);
            delete proxy;

            break;
        }

        default: {
            std::cerr << "[FreeGrok] Unknown command " << static_cast<int>(type) << " sent" << std::endl;

            break;
        }
    }
}

void GrokClient::go_to_message() {
    loop->recv(socket, sizeof(uint16_t),
               TYPE_GOT);
}

sock_t GrokClient::on_connect() {
    std::cout << "[FreeGrok] Connected to the freegrok server" << std::endl;

    return socket;
}

void GrokClient::post_connect(sock_t sock) {
    FStreamer streamer(loop, socket,
                       nullptr);
    streamer.send_header(GROK_PING, 0);
    this->go_to_message();
}

void GrokClient::on_received(state_t _state, ReadItem &item) {
    auto state = static_cast<PacketState>(_state);

    switch (state) {
        case TYPE_GOT: {
            BufferReader reader(item.buffer);
            auto data = reader.read<uint8_t>();
            auto type = GROK_GET_TYPE(data);
            auto length_chunk = reader.read<uint8_t>();

            if (GROK_IS_SHORT(data)) {
                if (length_chunk == 0) {
                    FStreamer streamer(loop, socket,
                                       nullptr);
                    this->process_message(type, length_chunk,
                                          streamer);

                    break;
                }

                auto meta = (static_cast<uint64_t>(data) << 32u) | static_cast<uint32_t>(length_chunk);
                loop->recv(socket, length_chunk, BUFFER_GOT,
                           meta);
            } else {
                auto meta = (static_cast<uint64_t>(data) << 32u) | static_cast<uint32_t>(length_chunk);

                loop->recv(socket, 3u,
                           HEADER_GOT, meta);
            }

            return;
        }

        case PacketState::HEADER_GOT: {
            auto meta = item.u64;
            auto data = static_cast<uint8_t>(meta >> 32u);
            auto length_chunk = static_cast<uint8_t>(meta & 0xffffffffu);
            unsigned char length_buf[sizeof(uint32_t)];
            length_buf[0] = length_chunk;
            memcpy(reinterpret_cast<char *>(length_buf + 1u),
                   item.buffer, sizeof(uint32_t) - sizeof(uint8_t));

            auto length = bytes_to_int<uint32_t>(reinterpret_cast<char *>(length_buf));
            auto type = GROK_GET_TYPE(data);

            if (length == 0) {
                FStreamer streamer(loop, socket, nullptr);
                this->process_message(type, length, streamer);

                break;
            }

            auto merged = (static_cast<uint64_t>(data) << 32u) | length;
            loop->recv(item.fd, length,
                       PacketState::BUFFER_GOT,
                       merged);

            return;
        }

        case PacketState::BUFFER_GOT: {
            auto merged = item.u64;
            auto data = static_cast<uint8_t>(merged >> 32u);
            auto type = GROK_GET_TYPE(data);
            auto length = static_cast<uint32_t>(merged & 0xffffffffu);

            FStreamer streamer(loop, item.fd,
                               item.buffer);

            this->process_message(type, length, streamer);

            break;
        }
    }

    this->go_to_message();
}

void GrokClient::on_disconnect(sock_t sock) {
    std::cout << "[FreeGrok] Disconnected" << std::endl;
}

/// IClient

void GrokClient::redirect_packet(c_id_t client_id, char *buffer, size_t length) {
    FStreamer streamer(loop, socket, nullptr);
    streamer.send_header(GROK_PACKET, sizeof(c_id_t)+length);
    streamer.send(client_id);
    streamer.send(buffer, length);
}

void GrokClient::send_disconnected(c_id_t client_id) {
    FStreamer streamer(loop, socket, nullptr);
    streamer.send_header(GROK_DISCONNECT, sizeof(c_id_t));
    streamer.send(client_id);
}
