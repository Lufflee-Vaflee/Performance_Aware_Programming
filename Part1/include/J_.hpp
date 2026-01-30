#pragma once

#include "LT.hpp"
#include "decode.hpp"

#include <unordered_map>
#include <optional>

namespace decode {

class label_gen {
   public:
    label_gen(stream_it_t begin) :
        m_begin(begin) {}

    std::optional<std::size_t> check_for_label(stream_it_t abs) {
        std::size_t pos = abs - m_begin;
        auto it = m_table.find(pos);
        if(it == m_table.end()) {
            return {};
        }

        return it->second;
    }

    void reg_label(stream_it_t abs) {
        std::size_t pos = abs - m_begin;
        m_table[pos] = m_label_inc;
        m_label_inc++;
    }

   private:
    std::unordered_map<std::size_t, std::size_t> m_table;
    std::size_t m_label_inc = 1;
    stream_it_t m_begin;
};

template<opcode::ID c_id = 256>
inline decode_inst_t decode_conditional_j(stream_it_t begin, stream_it_t end) {
    using namespace opcode;

    ID id = c_id;
    if constexpr (c_id == 256) {
        constexpr ID arr[16] = {
            JO ,
            JNO,
            JB ,
            JAE,
            JZ ,
            JNE,
            JBE,
            JA ,
            JS ,
            JNS,
            JP ,
            JPO,
            JL ,
            JNL,
            JLE,
            JG 
        };

        uint8_t code = *begin & 0b00001111;
        id = arr[code];
    }

    immediate im = *(begin + 1);
    return { { id, im, noarg{} }, 2};
}

}

