#pragma once

#include "decode.hpp"

namespace decode::ADD {

struct RM_R {
    W m_W : 1;
    D m_D : 1;
    uint8_t pad : 6; //opcode - unused

    uint8_t m_RM : 3;
    REG m_REG : 3;
    MOD m_MOD : 2;
};

static_assert(sizeof(RM_R) == 2);

decode_inst_t decode_ADD_RM_R(stream_it_t begin, stream_it_t end);
decode_inst_t decode_ADD_I_RM(stream_it_t begin, stream_it_t end);
decode_inst_t decode_ADD_I_A(stream_it_t begin, stream_it_t end);

}

