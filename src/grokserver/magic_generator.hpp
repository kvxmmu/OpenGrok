//
// Created by nero on 16.06.2021.
//

#ifndef GROKPP_MAGIC_GENERATOR_HPP
#define GROKPP_MAGIC_GENERATOR_HPP


#include <loop/platform.h>
#include <random>
#include <cstring>

#include <fstream>


size_t generate_magic(char *out_buffer, size_t max_length,
                      const std::string &magic) {
    /*
     * Generate magic
     */

    if (!magic.empty()) {
        memcpy(out_buffer, magic.data(), magic.size());

        return magic.size();
    }

    const static char *ascii_seq = "qwertyuiop[]asdfghjkl;'zxcvbnm,./QWERTYUIOP{}ASDFGH"
                                   "JKL:\"ZXCVBNM<>?";
    const static auto ascii_seq_len = strlen(ascii_seq);

    std::mt19937 mt(time(nullptr));
    size_t len = mt() % max_length + 10;
#ifndef _PLATFORM_WINDOWS
    std::ifstream rand_block("/dev/urandom", std::ios::binary);
#endif

    for (size_t pos = 0; pos < len; pos++) {
#ifdef _PLATFORM_WINDOWS
        out_buffer[pos] = ascii_seq[mt() % ascii_seq_len];
#else
        char rand_pos;
        rand_block.readsome(&rand_pos, sizeof(char));
        out_buffer[pos] = ascii_seq[rand_pos % ascii_seq_len];
#endif
    }

#ifndef _PLATFORM_WINDOWS
    rand_block.close();
#endif

    return len;
}

#endif //GROKPP_MAGIC_GENERATOR_HPP
