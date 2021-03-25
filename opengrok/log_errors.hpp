//
// Created by kvxmmu on 3/25/21.
//

#ifndef GROKLOOP_LOG_ERRORS_HPP
#define GROKLOOP_LOG_ERRORS_HPP

#include <stdexcept>

namespace Exceptions {
    class FileOpeningError : public std::runtime_error {
    public:
        std::string filename;

        explicit FileOpeningError(const char *_filename) : filename(_filename), std::runtime_error("Error opening file") {

        }
    };
}

#endif //GROKLOOP_LOG_ERRORS_HPP
