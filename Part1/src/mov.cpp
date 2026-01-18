#include "mov.hpp"

#include <sstream>
#include <utility>

#include <iostream>

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

    //std::unreachable();
    throw "shit2";
}

std::string decode_ADDR_CALC(ADDR_CALC const& calc) noexcept {
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

    //std::unreachable();
    throw "shit3";
}

decode_inst_t decode_MOV_RM_R(stream_it_t begin, stream_it_t end) {
    std::stringstream str;
    str << "MOV ";
    RM_R inst;

    std::string LHS;
    std::string RHS;

    uint16_t displacment;

    raw_deserialize<RM_R>(inst, begin, end);
    LHS = decode_REG(inst.m_REG, inst.m_W);

    switch (inst.m_MOD) {
        case MOD::REGISTER:
            RHS = decode_REG(REG(inst.m_RM), inst.m_W);
            if(!inst.m_D)
                std::swap(LHS, RHS);

            str << LHS << ", " << RHS;
            return { str.str(), 2 };
        case MOD::MEM_NO_DISPLACMENT:
            RHS = decode_ADDR_CALC(ADDR_CALC(inst.m_RM));
            std::cout << RHS << "\n\n";
            if(RHS == "BP") {
                raw_deserialize<uint16_t>(displacment, begin + 2, end);
                str << LHS << ", [" << displacment << ']' << std::endl;
                return {str.str(), 4 };
            }
            str << LHS << ", [" << RHS << ']';
            return {str.str(), 2 };
        case MOD::MEM_8_DISPLACMENT:
            RHS = decode_ADDR_CALC(ADDR_CALC(inst.m_RM));
            displacment = *(begin + 2);
            str << LHS << ", [" << RHS << " + " << displacment << ']';
            return {str.str(), 3 };
        case MOD::MEM_16_DISPLACMENT:
            RHS = decode_ADDR_CALC(ADDR_CALC(inst.m_RM));
            raw_deserialize<uint16_t>(displacment, begin + 2, end);
            str << LHS << ", [" << RHS << " + " << displacment << ']';
            return {str.str(), 4 };
    }

    throw "shit1";
}

decode_inst_t decode_MOV_RM(stream_it_t begin, stream_it_t end) {

}

}
