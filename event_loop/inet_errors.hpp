//
// Created by kvxmmu on 3/25/21.
//

#ifndef GROKLOOP_INET_ERRORS_HPP
#define GROKLOOP_INET_ERRORS_HPP

#include <stdexcept>

namespace Exceptions {
    class NotEnoughBytes : std::runtime_error {
    public:
        int required;
        int received;

        NotEnoughBytes(int _required, int _received) : required(_required), received(_received),
                std::runtime_error("Not enough bytes received") {

        }
    };
}


#endif //GROKLOOP_INET_ERRORS_HPP
