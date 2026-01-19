#pragma once

#include <cstdint>

#include "decode.hpp"

namespace decode::MOV {

struct I_R { 
    REG m_REG : 3;
    W m_W : 1;
    uint8_t pad : 4; //opcode - unused
};

static_assert(sizeof(I_R) == 1);

struct A { 
    W m_W : 1;
    D m_D : 1;
    uint8_t pad : 6; //opcode - unused
};

static_assert(sizeof(A) == 1);

decode_inst_t decode_MOV_RM_R(stream_it_t begin, stream_it_t end);
decode_inst_t decode_MOV_I_RM(stream_it_t begin, stream_it_t end);
decode_inst_t decode_MOV_I_R(stream_it_t begin, stream_it_t end);
decode_inst_t decode_MOV_M_A(stream_it_t begin, stream_it_t end);

}
