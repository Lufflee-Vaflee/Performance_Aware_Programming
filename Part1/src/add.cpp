#include "add.hpp"

namespace decode::ADD {

decode_inst_t decode_ADD_RM_R(stream_it_t begin, stream_it_t end) {
    std::stringstream str;
    RM_R_D inst;
    std::string LHS;


    raw_deserialize<RM_R_D>(inst, begin, end);
    LHS = decode_REG(inst.m_REG, inst.m_W);
    auto [ RHS, size ] = decode_DISPLACMENT(inst, begin, end);

    if(!inst.m_D) std::swap(LHS, RHS);
    str << "ADD " << LHS << ", " << RHS;
    return { str.str(), size + 2 };
}

decode_inst_t decode_ADD_I_RM(stream_it_t begin, stream_it_t end) {
    std::stringstream str;
    RM_R_S inst;

    raw_deserialize<RM_R_S>(inst, begin, end);
    auto [ LHS, size_LHS ] = decode_DISPLACMENT(inst, begin, end);
    auto [ RHS, size_RHS ] = decode_WDATA<int16_t>(inst.m_W, inst.m_S, begin + size_LHS + sizeof(RM_R_D), end);

    str << "ADD " << LHS << ", " << format_immediate(RHS, inst.m_W);
    return { str.str(), sizeof(RM_R_D) + size_LHS + size_RHS };
}

decode_inst_t decode_ADD_I_A(stream_it_t begin, stream_it_t end) {
    std::stringstream str;
    A inst;

    raw_deserialize<A>(inst, begin, end);
    auto LHS = inst.m_W ? "AX" : "AL";
    auto [ RHS, size_RHS ] = decode_WDATA<int16_t>(inst.m_W, 0, begin + sizeof(A), end);

    str << "ADD " << LHS << ", " << RHS;
    return { str.str(), sizeof(A) + size_RHS };
}

decode_inst_t decode_SUB_RM_R(stream_it_t begin, stream_it_t end) {
    std::stringstream str;
    RM_R_D inst;
    std::string LHS;


    raw_deserialize<RM_R_D>(inst, begin, end);
    LHS = decode_REG(inst.m_REG, inst.m_W);
    auto [ RHS, size ] = decode_DISPLACMENT(inst, begin, end);

    if(!inst.m_D) std::swap(LHS, RHS);
    str << "SUB " << LHS << ", " << RHS;
    return { str.str(), size + 2 };
}

decode_inst_t decode_SUB_I_RM(stream_it_t begin, stream_it_t end) {
    std::stringstream str;
    RM_R_S inst;

    raw_deserialize<RM_R_S>(inst, begin, end);
    auto [ LHS, size_LHS ] = decode_DISPLACMENT(inst, begin, end);
    auto [ RHS, size_RHS ] = decode_WDATA<int16_t>(inst.m_W, inst.m_S, begin + size_LHS + sizeof(RM_R_D), end);

    str << "SUB " << LHS << ", " << format_immediate(RHS, inst.m_W);
    return { str.str(), sizeof(RM_R_D) + size_LHS + size_RHS };
}

decode_inst_t decode_SUB_I_A(stream_it_t begin, stream_it_t end) {
    std::stringstream str;
    A inst;

    raw_deserialize<A>(inst, begin, end);
    auto LHS = inst.m_W ? "AX" : "AL";
    auto [ RHS, size_RHS ] = decode_WDATA<int16_t>(inst.m_W, 0, begin + sizeof(A), end);

    str << "SUB " << LHS << ", " << RHS;
    return { str.str(), sizeof(A) + size_RHS };
}

}

