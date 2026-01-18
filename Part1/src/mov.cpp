#include "mov.hpp"

#include <sstream>
#include <utility>

namespace decode::MOV {

std::string decode_REG(REG const& reg, W const& w) noexcept {
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

decode_inst_t decode_MOV_RM_R(stream_it_t begin, stream_it_t end) {
    std::stringstream str;
    str << "MOV ";
    RM_R inst;
    raw_deserialize<RM_R>(inst, begin, end);
    std::string LHS = decode_REG(inst.m_REG, inst.m_W);
    std::string RHS = decode_REG(inst.m_RM, inst.m_W);
    if(!inst.m_D) std::swap(LHS, RHS);

    str << LHS << ", " << RHS;
    return { str.str(), 2 };
}

}
