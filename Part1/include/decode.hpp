#pragma once

#include <string>
#include <utility>
#include <vector>
#include <cassert>
#include <sstream>
#include <cstdint>

#include "opcode.hpp"

namespace decode {

using data_stream_t = std::vector<char>;
using stream_it_t = std::vector<char>::iterator;

opcode::decoded decode(stream_it_t& begin, stream_it_t end);

//opcode_stream_t decode(data_stream_t&);
data_stream_t load_input_stream(std::fstream& stream);

using decode_inst_t = std::pair<opcode::decoded, int>;
using decode_arg_t =  std::pair<opcode::arg_t, int>;

inline std::string format_displacment(int16_t displacment) {
    std::stringstream result;
    result << (displacment > 0 ? " + " : " - ") << std::abs(displacment);
    return result.str();
}


inline std::string format_immediate(int16_t immediate, opcode::W w) {
    std::stringstream result;
    result << (w ? "word " : "byte ") << immediate;
    return result.str();
}

inline std::string decode_REG(opcode::REG const& reg, opcode::W const& w) noexcept {
    using namespace opcode;
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

inline std::string decode_ADDR_CALC(opcode::DIS const& calc) noexcept {
    using namespace opcode;
    switch(calc) {
        case DIS::BX_SI : return "BX + SI";
        case DIS::BX_DI : return "BX + DI";
        case DIS::BP_SI : return "BP + SI";
        case DIS::BP_DI : return "BP + DI";
        case DIS::SI    : return "SI";
        case DIS::DI    : return "DI";
        case DIS::DIRECT: return "BP";
        case DIS::BX    : return "BX";
    }

    std::unreachable();
}

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

inline std::pair<opcode::arg_t, int> decode_RM(opcode::unpacked_bitmap bitmap, stream_it_t begin, stream_it_t end) {
    using namespace opcode;
    displacment_mem ARG;

    assert(bitmap.mod.has_value());
    assert(bitmap.rm.has_value());
    switch (*bitmap.mod) {
        case MOD::REGISTER:
            return { REG(*(bitmap.rm)), 0 };
        case MOD::MEM_NO_DISPLACMENT:
            if((DIS(*(bitmap.rm))) == DIS::DIRECT) {
                direct_addr addr;
                raw_deserialize<uint16_t>(addr, begin, end);
                return { addr , 2 };
            }

            return { DIS(*(bitmap.rm)), 0 };
        case MOD::MEM_8_DISPLACMENT:
            ARG = { *(begin), DIS(*(bitmap.rm)) };
            return { ARG, 1 };
        case MOD::MEM_16_DISPLACMENT:
            raw_deserialize<int16_t>(ARG.displacment, begin, end);
            ARG.reg = *(bitmap.rm);
            return { ARG, 2 };
    }
}

inline std::pair<opcode::arg_t, int> decode_WDATA(opcode::unpacked_bitmap bitmap, stream_it_t begin, stream_it_t end) {
    using namespace opcode;
    immediate arg = 0;

    assert(bitmap.w.has_value());
    auto w = *(bitmap.w);
    auto s = bitmap.s.has_value() ? *(bitmap.s) : 0;

    int S_W = s;
    S_W <<= 1;
    S_W += w;

    switch(S_W) {
    case 0:
        return { *(begin), 1 };
    case 1:
        raw_deserialize<immediate>(arg, begin, end);
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

inline std::pair<opcode::arg_t, int> decode_reg(opcode::unpacked_bitmap bitmap, stream_it_t begin, stream_it_t end) {
    assert(bitmap.reg.has_value());
    return { *(bitmap.reg), 0 };
}

inline std::pair<opcode::arg_t, int> decode_AX(opcode::unpacked_bitmap bitmap, stream_it_t begin, stream_it_t end) {
    return { opcode::REG::AL_AX, 0 };
}

template <typename F>
concept Delegator =
    std::invocable<
        F,
        opcode::unpacked_bitmap,
        stream_it_t,
        stream_it_t
    > &&
    std::same_as<
        std::invoke_result_t<
        F,
        opcode::unpacked_bitmap,
        stream_it_t,
        stream_it_t
        >,
        std::pair<opcode::arg_t, int>
    >;

template <opcode::ID id, auto D1, auto D2>
requires(
    Delegator<decltype(D1)> &&
    Delegator<decltype(D2)>)
decode_inst_t generalized_decode(stream_it_t begin, stream_it_t end) {
    using bitmask_t = opcode::get_bitmap<id>::t;
    bitmask_t packed;
    raw_deserialize<bitmask_t>(packed, begin, end);

    opcode::unpacked_bitmap unpacked = unpack_bitmap<bitmask_t>(packed);
    begin += sizeof(bitmask_t);
    decode_arg_t LHS_d = D1(unpacked, begin, end); 
    begin += LHS_d.second;
    decode_arg_t RHS_d = D2(unpacked, begin, end);

    int size = sizeof(bitmask_t) + LHS_d.second + RHS_d.second;
    if constexpr (opcode::details::has_d<bitmask_t>) {
        std::swap(LHS_d.first, RHS_d.first);
    }

    return { { id, LHS_d.first, RHS_d.first}, size };
}

}
