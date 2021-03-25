//
// Created by kvxmmu on 3/25/21.
//

#ifndef GROKLOOP_ID_POOL_HPP
#define GROKLOOP_ID_POOL_HPP

#include <deque>

template <typename T>
class IDPool {
public:
    std::deque<T> ids;
    T last_id;

    typedef T value_type;

    explicit IDPool(T start = 1) : last_id(start) {

    }

    void remove_id(T id) {
        this->ids.push_back(id);
    }

    T get_id() {
        if (!this->ids.empty()) {
            auto front_id = this->ids.front();
            this->ids.pop_front();

            return front_id;
        }

        return this->last_id++;
    }
};

#endif //GROKLOOP_ID_POOL_HPP
