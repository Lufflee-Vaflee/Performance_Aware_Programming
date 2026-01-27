#include "JT.hpp"

#include <sstream>
#include <utility>

#include <iostream>

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

decode_inst_t RM_R(stream_it_t begin, stream_it_t end) {
    std::stringstream str;
    RM_R_D inst;
    std::string LHS;

    raw_deserialize<RM_R_D>(inst, begin, end);
    LHS = decode_REG(inst.m_REG, inst.m_W);
    auto [ RHS, size ] = decode_DISPLACMENT(inst, begin, end);

    if(!inst.m_D) std::swap(LHS, RHS);
    str << "MOV " << LHS << ", " << RHS;
    return { str.str(), size + 2 };
}

decode_inst_t I_RM(stream_it_t begin, stream_it_t end) {
    std::stringstream str;
    RM_R_D inst;

    raw_deserialize<RM_R_D>(inst, begin, end);
    auto [ LHS, size_LHS ] = decode_DISPLACMENT(inst, begin, end);
    auto [ RHS, size_RHS ] = decode_WDATA<int16_t>(inst.m_W, 0, begin + size_LHS + sizeof(RM_R_D), end);

    str << "MOV " << LHS << ", " << format_immediate(RHS, inst.m_W);
    return { str.str(), sizeof(RM_R_D) + size_LHS + size_RHS };
}


decode_inst_t DI_R(stream_it_t begin, stream_it_t end) {
    std::stringstream str;
    I_R inst;

    raw_deserialize<I_R>(inst, begin, end);
    auto LHS = decode_REG(inst.m_REG, inst.m_W);
    auto [ RHS, size_RHS ] = decode_WDATA<int16_t>(inst.m_W, 0, begin + sizeof(I_R), end);

    str << "MOV " << LHS << ", " << RHS;
    return { str.str(), sizeof(I_R) + size_RHS };
}


decode_inst_t M_A(stream_it_t begin, stream_it_t end) {
    std::stringstream str;
    A inst;

    raw_deserialize<A>(inst, begin, end);
    std::string LHS = "AX";
    auto [ RHS, size_RHS ] = decode_WDATA<uint16_t>(inst.m_W, 0, begin + sizeof(A), end);
    str << '[' << RHS << "]";
    std::string RHS_str = str.str();
    str.str("");

    if(inst.m_D) {
        std::swap(LHS, RHS_str);
    }

    str << "MOV " << LHS << ", " << RHS_str;
    return { str.str(), sizeof(A) + size_RHS };
}

}

