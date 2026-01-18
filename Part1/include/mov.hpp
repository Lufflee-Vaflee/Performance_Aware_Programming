#pragma once

#include <cstdint>

#include "decode.hpp"

namespace decode::MOV {

using D = bool;

//Word/byte
using W = bool;

//Mode encoding
enum class MOD : uint8_t {
    MEM_NO_DISPLACMENT = 0b00,
    MEM_8_DISPLACMENT = 0b01,
    MEM_16_DISPLACMENT = 0b10,
    REGISTER = 0b11
};

enum class REG : uint8_t {
    AL_AX = 0b000,
    CL_CX = 0b001,
    DL_DX = 0b010,
    BL_BX = 0b011,
    AH_SP = 0b100,
    CH_BP = 0b101,
    DH_SI = 0b110,
    BH_DI = 0b111,
};

enum class ADDR_CALC : uint8_t {
    BX_SI = 0b000,
    BX_DI = 0b001,
    BP_SI = 0b010,
    BP_DI = 0b011,
    SI = 0b100,
    DI = 0b101,
    DIRECT = 0b110,
    BX = 0b111,
};

struct RM_R { 
    W m_W : 1;
    D m_D : 1;
    uint8_t pad : 6; //opcode - unused

    uint8_t m_RM : 3;
    REG m_REG : 3;
    MOD m_MOD : 2;
};

static_assert(sizeof(RM_R) == 2);

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
