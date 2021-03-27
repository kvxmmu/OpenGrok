//
// Created by kvxmmu on 3/27/21.
//

#ifndef OPENGROK_ID_POOL_HPP
#define OPENGROK_ID_POOL_HPP

#include <deque>
#include <unordered_set>

template <typename seq_t>
class IDPoolInterface {
public:
    typedef seq_t sequence_type;

    seq_t last;

    explicit IDPoolInterface(seq_t start_value = 0) : last(start_value) {

    }

    virtual void push_id(seq_t id) = 0;
    virtual seq_t pop_id() = 0;
    virtual bool empty() = 0;
    virtual seq_t get_new_id() = 0;

    void free_id(seq_t id) {
        this->push_id(id);
    }

    seq_t get_id() {
        if (!this->empty()) {
            seq_t sequence = this->pop_id();

            return sequence;
        }

        return this->get_new_id();
    }

    virtual ~IDPoolInterface() = default;
};

template <typename seq_t>
class ConsistentIDPool : public IDPoolInterface<seq_t> {
public:
    std::deque<seq_t> ids;

    void push_id(seq_t id) override {
        this->ids.push_back(id);
    }

    seq_t pop_id() {
        auto id = this->ids.front();
        this->ids.pop_front();

        return id;
    }

    bool empty() override {
        return this->ids.empty();
    }

    seq_t get_new_id() override {
        return this->last++;
    }
};

template <typename seq_t>
class OrderedIDPool : public IDPoolInterface<seq_t> {
public:
    std::unordered_set<seq_t> ids;

    void push_id(seq_t id) override {
        this->ids.insert(id);
    }

    seq_t pop_id() {
        seq_t id = *this->ids.begin();
        this->ids.erase(id);

        return id;
    }

    bool empty() override {
        return this->ids.empty();
    }

    seq_t get_new_id() override {
        return this->last++;
    }
};

#endif //OPENGROK_ID_POOL_HPP
