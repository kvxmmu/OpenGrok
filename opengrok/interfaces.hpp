//
// Created by kvxmmu on 3/25/21.
//

#ifndef GROKLOOP_INTERFACES_HPP
#define GROKLOOP_INTERFACES_HPP

#include "id_pool.hpp"

#include <unordered_map>

using GIDPool = IDPool<uint64_t>;
class IProxyServer;

struct client_pair_t {
    int fd;
    IProxyServer *server;

    client_pair_t() = default;

    client_pair_t(int _fd, IProxyServer *_server) : fd(_fd), server(_server) {

    }
};

class IServerDatabase {
public:
    std::unordered_map<GIDPool::value_type, client_pair_t> clients_database;
    std::unordered_map<int, GIDPool::value_type> client_fds;

    GIDPool id_pool;

    GIDPool::value_type link_client(int fd, IProxyServer *server) {
        auto id = this->id_pool.get_id();

        this->clients_database[id] = client_pair_t(fd, server);
        this->client_fds[fd] = id;

        return id;
    }

    void unlink_client(GIDPool::value_type id) {
        this->client_fds.erase(this->get_client_fd(id));
        this->clients_database.erase(id);

        this->id_pool.remove_id(id);
    }

    bool has_client(GIDPool::value_type id) {
        return this->clients_database.find(id) != this->clients_database.end();
    }

    int get_client_fd(GIDPool::value_type id) {
        return this->clients_database.at(id).fd;
    }

    IProxyServer *get_client_server(GIDPool::value_type id) {
        return this->clients_database.at(id).server;
    }

    GIDPool::value_type get_client_id(int fd) {
        return this->client_fds.at(fd);
    }
};

class IServerForward {
public:
    virtual void connect(int dest, GIDPool::value_type client_id) = 0; // ко мне подключаются
    virtual void forward(int dest,
                         GIDPool::value_type client_id, char *buffer, uint32_t length) = 0;
    virtual void disconnect(int dest, GIDPool::value_type client_id) = 0; // от меня кто-то отключается
};

class IServer : public IServerDatabase, public IServerForward {
public:

};

class IProxyServer {
public:
    IServer *server;

    explicit IProxyServer(IServer *_server) : server(_server) {

    }

    virtual void forward(GIDPool::value_type client_id,
            char *buffer, uint32_t length) = 0;
    virtual void disconnect(GIDPool::value_type client_id) = 0;
};



#endif //GROKLOOP_INTERFACES_HPP
