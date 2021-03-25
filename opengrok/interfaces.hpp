//
// Created by kvxmmu on 3/25/21.
//

#ifndef GROKLOOP_INTERFACES_HPP
#define GROKLOOP_INTERFACES_HPP

#include "id_pool.hpp"

#include <unordered_map>

using GIDPool = IDPool<uint64_t>;

class IServerDatabase {
public:
    std::unordered_map<GIDPool::value_type, int> clients_database;
    GIDPool id_pool;

    GIDPool::value_type link_client(int fd) {
        auto id = this->id_pool.get_id();

        this->clients_database[id] = fd;

        return id;
    }

    void unlink_client(GIDPool::value_type id) {
        this->clients_database.erase(id);
        this->id_pool.remove_id(id);
    }

    bool has_client(GIDPool::value_type id) {
        return this->clients_database.find(id) != this->clients_database.end();
    }

    int get_client_fd(GIDPool::value_type id) {
        return this->clients_database.at(id);
    }
};

class IServerForward {
public:
    virtual void connect(GIDPool::value_type client_id) = 0; // ко мне подключаются
    virtual void forward(GIDPool::value_type client_id, char *buffer,
            uint32_t length) = 0;
    virtual void disconnect(GIDPool::value_type client_id) = 0; // от меня кто-то отключается
};

class IServer : public IServerDatabase, public IServerForward {
public:

};

class IProxyServer {
public:
    IServer *server;

    explicit IProxyServer(IServer *_server) : server(_server) {

    }

    virtual void forward(GIDPool::value_type client_id, char *buffer, uint32_t length) = 0;
    virtual void disconnect(GIDPool::value_type client_id) = 0;
};



#endif //GROKLOOP_INTERFACES_HPP
