//
// Created by kvxmmu on 3/27/21.
//

#ifndef OPENGROK_WRAPPER_HPP
#define OPENGROK_WRAPPER_HPP

#include <cameleo/loop.hpp>
#include <cameleo/abstract.hpp>

#include <grokbuffer/bytebuffer.hpp>

#ifndef MAX_LISTEN
#   define MAX_LISTEN 64u
#endif

namespace GrokBufferProtocol {
    using namespace Cameleo;

    class Server : public IObserver {
    public:
        const uint16_t listen_port;
        const in_addr_t listen_addr;

        sockaddr_in addr{};

        typedef uint8_t type_t;
        typedef uint32_t length_t;

        typedef std::function<void(BufferReader &, int,
                type_t, length_t)> packet_callback_t;
        typedef std::function<void(int)> motd_callback_t;
        typedef std::function<void(int)> disconnect_callback_t;

        Endianess endianess;

        motd_callback_t motd_callback = nullptr;
        packet_callback_t packet_callback = nullptr;
        disconnect_callback_t disconnect_callback = nullptr;

        uint32_t max_packet_length = 0xffffffffu;
        type_t error_type = 0u;

        const char *too_long_packet_description = "Too long pkt";
        uint8_t too_long_packet_error_code = 0u;

        Server(in_addr_t _listen_addr,
                uint16_t _listen_port, Endianess _endianess = Endianess::LITTLE) : listen_port(_listen_port), listen_addr(_listen_addr),
                                         IObserver(socket(AF_INET, SOCK_STREAM, 0), true), endianess(_endianess) {

        }

        //// Getters

        void type_getter(Future *future);
        void length_getter(Future *future, type_t type);
        void buffer_getter(Future *future, type_t type, length_t length);

        ////

        inline void fallback_to_type(int fd) {
            this->loop->recv(fd, sizeof(type_t), NONSTATIC_CALLBACK(this->type_getter));
        }

        void init(EventLoop *_loop) override;

        int on_connect() override;
        void post_connect(int fd) override;
        void on_disconnect(int fd) override;

        //// Responders

        void respond(int dest, type_t type, char *buffer, length_t length);
        void respond_error(int dest, uint8_t error_code, const char *error_description);

        ////
    };

    class Client : public IObserver {
    public:
        sockaddr_in caddr{};

        typedef uint8_t type_t;
        typedef uint32_t length_t;

        typedef std::function<void(BufferReader &, int,
                type_t, length_t)> packet_callback_t;
        typedef std::function<void(int)> motd_callback_t;
        typedef std::function<void(int)> disconnect_callback_t;

        Endianess endianess;

        motd_callback_t       motd_callback       = nullptr;
        packet_callback_t     packet_callback     = nullptr;
        disconnect_callback_t disconnect_callback = nullptr;

        Client(const char *ip,
                uint16_t port, Endianess _endianess = Endianess::LITTLE) : IObserver(socket(AF_INET, SOCK_STREAM, false), false),
                                                       endianess(_endianess) {
            caddr.sin_port = htons(port);
            caddr.sin_family = AF_INET;
            caddr.sin_addr.s_addr = inet_addr(ip);
        }

        void post_init() override {
            ::connect(this->sockfd, reinterpret_cast<sockaddr *>(&this->caddr),
                    sizeof(this->caddr));
        }

        //// Getters

        void type_getter(Future *future);
        void length_getter(Future *future, type_t type);
        void buffer_getter(Future *future, type_t type, length_t length);

        ////

        void fallback_to_type();

        int on_connect() override;
        void post_connect(int fd) override;
        void on_disconnect(int fd) override;

        void respond(type_t type, char *buffer, length_t length);
    };
}




#endif //OPENGROK_WRAPPER_HPP
