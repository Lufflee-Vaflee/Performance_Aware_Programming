#pragma once

#include "decode.hpp"

#include <unordered_map>
#include <sstream>
#include <limits>

namespace decode {

class conditional_jmp_table {
   public:
    decode_inst_t decode_conditional_jmp(const char* mnemonic, stream_it_t begin, stream_it_t abs, stream_it_t end) {
        str.str("");
        std::size_t label_num;
        int8_t displacment = *(abs + 1);
        std::size_t position = (abs - begin) + displacment;
        auto it = m_table.find(position);
        if(it == m_table.end()) {
            m_table[position] = label_increment;
            label_num = label_increment;
            label_increment++;
        } else {
            label_num = it->second;
        }

        str << mnemonic << " label" << label_num;
        return { str.str(), 2 };
    }

    std::size_t operator[](std::size_t pos) {
        auto it = m_table.find(pos);
        if(it == m_table.end()) {
            return std::numeric_limits<std::size_t>::max();
        }

        return it->second;
    }

   private:
    std::unordered_map<std::size_t, std::size_t> m_table;
    std::size_t label_increment = 1;
    std::stringstream str;
};


}

