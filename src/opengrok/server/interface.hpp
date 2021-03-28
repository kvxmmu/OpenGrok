//
// Created by kvxmmu on 3/27/21.
//

#ifndef OPENGROK_INTERFACE_HPP
#define OPENGROK_INTERFACE_HPP

#include <opengrok/id_pool.hpp>
#include <unordered_map>

#ifndef _OPENGROK_DEBUG
#   define OPENGROK_PROXY_PORT 6560
#else
#   define OPENGROK_PROXY_PORT 0
#endif

class IProxyServer {
public:
    virtual int get_listening_port() = 0;

    virtual void forward(client_id_t client_id, char *buffer, size_t length) = 0;
    virtual void disconnect(client_id_t client_id) = 0;

    virtual void shutdown() = 0;

    virtual ~IProxyServer() = default;
};

class ClientDatabaseMixin {
public:
    IDPool id_pool;

    std::unordered_map<int, client_id_t> fd_to_id;
    std::unordered_map<client_id_t, int> id_to_fd;

    client_id_t link_client(int fd) {
        auto id = this->id_pool.get_id();

        this->fd_to_id[fd] = id;
        this->id_to_fd[id] = fd;

        return id;
    }

    void unlink_by_id(client_id_t client_id) {
        auto client_fd = this->id_to_fd.at(client_id);

        this->unlink(client_id, client_fd);
    }

    void unlink_by_fd(int fd) {
        auto client_id = this->fd_to_id.at(fd);

        this->unlink(client_id, fd);
    }

    int get_fd_by_id(client_id_t client_id) {
        return this->id_to_fd.at(client_id);
    }

    client_id_t get_id_by_fd(int fd) {
        return this->fd_to_id.at(fd);
    }

    void unlink(client_id_t client_id, int client_fd) {
        this->fd_to_id.erase(client_fd);
        this->id_to_fd.erase(client_id);

        this->id_pool.free_id(client_id);
    }
};

class IMainServer : public ClientDatabaseMixin {
public:
    virtual void forward(int dest, client_id_t client_id,
            char *buffer, size_t length) = 0;
    virtual void connect(int dest, client_id_t client_id) = 0;
    virtual void disconnect(int dest, client_id_t client_id) = 0;


    virtual ~IMainServer() = default;
};

#endif //OPENGROK_INTERFACE_HPP
