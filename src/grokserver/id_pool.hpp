//
// Created by nero on 12.06.2021.
//

#ifndef GROKPP_ID_POOL_HPP
#define GROKPP_ID_POOL_HPP

#include <deque>
#include <type_traits>

template <typename T>
class IdPool {
public:
    typedef typename std::enable_if<std::is_integral<T>::value, T>::type id_seq_t;

    id_seq_t last;
    std::deque<id_seq_t> ids;

    explicit
    IdPool(id_seq_t start_from = 0) : last(start_from) {

    }

    id_seq_t get_id() {
        if (ids.empty()) {
            return last++;
        }

        auto ret_id = ids.front();
        ids.pop_front();

        return ret_id;
    }

    void remove_id(id_seq_t id) {
        ids.push_back(id);
    }
};

#endif //GROKPP_ID_POOL_HPP
