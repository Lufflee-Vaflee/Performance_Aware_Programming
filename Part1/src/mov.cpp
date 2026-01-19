#include "mov.hpp"

#include <sstream>
#include <utility>

#include <iostream>

namespace decode::MOV {


std::string format_immediate(int16_t immediate, W w) {
    std::stringstream result;
    result << (w ? "word " : "byte ") << immediate;
    return result.str();
}

decode_inst_t decode_MOV_RM_R(stream_it_t begin, stream_it_t end) {
    std::stringstream str;
    RM_R inst;
    std::string LHS;

    raw_deserialize<RM_R>(inst, begin, end);
    LHS = decode_REG(inst.m_REG, inst.m_W);
    auto [ RHS, size ] = decode_DISPLACMENT(inst, begin, end);

    if(!inst.m_D) std::swap(LHS, RHS);
    str << "MOV " << LHS << ", " << RHS;
    return { str.str(), size + 2 };
}

decode_inst_t decode_MOV_I_RM(stream_it_t begin, stream_it_t end) {
    std::stringstream str;
    RM_R inst;

    raw_deserialize<RM_R>(inst, begin, end);
    auto [ LHS, size_LHS ] = decode_DISPLACMENT(inst, begin, end);
    auto [ RHS, size_RHS ] = decode_WDATA<int16_t>(inst.m_W, begin + size_LHS + sizeof(RM_R), end);

    str << "MOV " << LHS << ", " << format_immediate(RHS, inst.m_W);
    return { str.str(), sizeof(RM_R) + size_LHS + size_RHS };
}


decode_inst_t decode_MOV_I_R(stream_it_t begin, stream_it_t end) {
    std::stringstream str;
    I_R inst;

    raw_deserialize<I_R>(inst, begin, end);
    auto LHS = decode_REG(inst.m_REG, inst.m_W);
    auto [ RHS, size_RHS ] = decode_WDATA<int16_t>(inst.m_W, begin + sizeof(I_R), end);

    str << "MOV " << LHS << ", " << RHS;
    return { str.str(), sizeof(I_R) + size_RHS };
}


decode_inst_t decode_MOV_M_A(stream_it_t begin, stream_it_t end) {
    std::stringstream str;
    std::stringstream str2;
    A inst;

    raw_deserialize<A>(inst, begin, end);
    std::string LHS = "AX";
    auto [ RHS, size_RHS ] = decode_WDATA<uint16_t>(inst.m_W, begin + sizeof(A), end);
    str2 << '[' << RHS << "]";
    std::string RHS_str = str2.str();

    if(inst.m_D) {
        std::swap(LHS, RHS_str);
    }

    str << "MOV " << LHS << ", " << RHS_str;
    return { str.str(), sizeof(A) + size_RHS };
}

}
