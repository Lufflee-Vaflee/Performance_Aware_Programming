#pragma once

#include <string>
#include <utility>
#include <vector>
#include <cassert>
#include <sstream>
#include <cstdint>

namespace decode {

using instr_stream_t = std::vector<char>;
using stream_it_t = std::vector<char>::iterator;

std::string decode(instr_stream_t&);
instr_stream_t load_input_stream(std::fstream& stream);

using decode_inst_t = std::pair<std::string, int>;

template<typename T>
//TODO add POD restriction
void raw_deserialize(T& dest, stream_it_t begin, stream_it_t end) {
    char* c_dest = reinterpret_cast<char*>(&dest);
    stream_it_t c_end = begin + sizeof(T);
    assert(end >= c_end);

    std::size_t i = 0;
    while(begin != c_end) {
        c_dest[i] = *begin;
        begin++;
        i++;
    }

    return;
}

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


using D = bool;
using W = bool;
using S = bool;

struct RM_R_D { 
    W m_W : 1;
    D m_D : 1;
    uint8_t pad : 6; //opcode - unused

    uint8_t m_RM : 3;
    REG m_REG : 3;
    MOD m_MOD : 2;
};

static_assert(sizeof(RM_R_D) == 2);

struct RM_R_S {
    W m_W : 1;
    S m_S : 1;
    uint8_t pad : 6; //opcode - unused

    uint8_t m_RM : 3;
    uint8_t pad2 : 3; // reg unused
    MOD m_MOD : 2;
};

static_assert(sizeof(RM_R_S) == 2);

struct A { 
    W m_W : 1;
    uint8_t pad : 7; //opcode - unused
};

static_assert(sizeof(A) == 1);

inline std::string format_displacment(int16_t displacment) {
    std::stringstream result;
    result << (displacment > 0 ? " + " : " - ") << std::abs(displacment);
    return result.str();
}


inline std::string format_immediate(int16_t immediate, W w) {
    std::stringstream result;
    result << (w ? "word " : "byte ") << immediate;
    return result.str();
}

inline std::string decode_REG(REG const& reg, W const& w) noexcept {
    switch(reg) {
        case REG::AL_AX: return w ? "AX" : "AL";
        case REG::CL_CX: return w ? "CX" : "CL";
        case REG::DL_DX: return w ? "DX" : "DL";
        case REG::BL_BX: return w ? "BX" : "BL";
        case REG::AH_SP: return w ? "SP" : "AH";
        case REG::CH_BP: return w ? "BP" : "CH";
        case REG::DH_SI: return w ? "SI" : "DH";
        case REG::BH_DI: return w ? "DI" : "BH";
    }

    std::unreachable();
}

inline std::string decode_ADDR_CALC(ADDR_CALC const& calc) noexcept {
    switch(calc) {
        case ADDR_CALC::BX_SI : return "BX + SI";
        case ADDR_CALC::BX_DI : return "BX + DI";
        case ADDR_CALC::BP_SI : return "BP + SI";
        case ADDR_CALC::BP_DI : return "BP + DI";
        case ADDR_CALC::SI    : return "SI";
        case ADDR_CALC::DI    : return "DI";
        case ADDR_CALC::DIRECT: return "BP";
        case ADDR_CALC::BX    : return "BX";
    }

    std::unreachable();
}


template<typename T>
decode_inst_t decode_DISPLACMENT(T inst, stream_it_t begin, stream_it_t end) {
    std::stringstream str;
    std::string ARG;
    int16_t displacment;

    switch (inst.m_MOD) {
        case MOD::REGISTER:
            ARG = decode_REG(REG(inst.m_RM), inst.m_W);
            return { ARG, 0 };
        case MOD::MEM_NO_DISPLACMENT:
            ARG = decode_ADDR_CALC(ADDR_CALC(inst.m_RM));
            if(ARG == "BP") {
                raw_deserialize<int16_t>(displacment, begin + 2, end);
                str << "[" << displacment << ']';
                ARG = str.str();
                return { ARG, 2 };
            }

            str << "[" << ARG << ']';
            ARG = str.str();
            return { ARG, 0 };
        case MOD::MEM_8_DISPLACMENT:
            ARG = decode_ADDR_CALC(ADDR_CALC(inst.m_RM));
            displacment = *(begin + 2);
            str << '[' << ARG << format_displacment(displacment) << ']';
            ARG = str.str();
            return { ARG, 1 };
        case MOD::MEM_16_DISPLACMENT:
            ARG = decode_ADDR_CALC(ADDR_CALC(inst.m_RM));
            raw_deserialize<int16_t>(displacment, begin + 2, end);
            str << '[' << ARG << format_displacment(displacment) << ']';
            ARG = str.str();
            return { ARG, 2 };
    }
}

template<typename T>
std::pair<T, int> decode_WDATA(W w, S s, stream_it_t begin, stream_it_t end) {
    int16_t arg = 0;

    int S_W = s;
    S_W <<= 1;
    S_W += w;

    switch(S_W) {
    case 0:
        return {*(begin), 1};
    case 1:
        raw_deserialize<int16_t>(arg, begin, end);
        return { arg , 2 };
    case 3:
        int8_t data = *(begin);
        if(data >= 0) {
            return { data, 1 };
        } else {
            arg = data;
            arg |= 0xFF00;
            return { arg, 1 };
        }
    }

    std::unreachable();
}

}
