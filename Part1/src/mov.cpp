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

std::string format_displacment(int16_t displacment) {
    std::stringstream result;
    result << (displacment > 0 ? " + " : " - ") << std::abs(displacment);
    return result.str();
}

std::string format_immediate(int16_t immediate, W w) {
    std::stringstream result;
    result << (w ? "word " : "byte ") << immediate;
    return result.str();
}

decode_inst_t decode_MOV_RM_RI_body(RM_R inst, stream_it_t begin, stream_it_t end) {
    std::stringstream str;
    std::string RHS;
    int16_t displacment;

    switch (inst.m_MOD) {
        case MOD::REGISTER:
            RHS = decode_REG(REG(inst.m_RM), inst.m_W);
            return { RHS, 2 };
        case MOD::MEM_NO_DISPLACMENT:
            RHS = decode_ADDR_CALC(ADDR_CALC(inst.m_RM));
            if(RHS == "BP") {
                raw_deserialize<int16_t>(displacment, begin + 2, end);
                str << "[" << displacment << ']';
                RHS = str.str();
                return { RHS, 4 };
            }

            str << "[" << RHS << ']';
            RHS = str.str();
            return { RHS, 2 };
        case MOD::MEM_8_DISPLACMENT:
            RHS = decode_ADDR_CALC(ADDR_CALC(inst.m_RM));
            displacment = *(begin + 2);
            str << '[' << RHS << format_displacment(displacment) << ']';
            RHS = str.str();
            return { RHS, 3 };
        case MOD::MEM_16_DISPLACMENT:
            RHS = decode_ADDR_CALC(ADDR_CALC(inst.m_RM));
            raw_deserialize<int16_t>(displacment, begin + 2, end);
            str << '[' << RHS << format_displacment(displacment) << ']';
            RHS = str.str();
            return { RHS, 4 };
    }

}

decode_inst_t decode_MOV_RM_R(stream_it_t begin, stream_it_t end) {
    std::stringstream str;
    RM_R inst;
    std::string LHS;


    raw_deserialize<RM_R>(inst, begin, end);
    LHS = decode_REG(inst.m_REG, inst.m_W);
    auto [ RHS, size ] = decode_MOV_RM_RI_body(inst, begin, end);

    if(!inst.m_D) std::swap(LHS, RHS);
    str << "MOV " << LHS << ", " << RHS;
    return {str.str(), size};
}

decode_inst_t decode_MOV_I_RM(stream_it_t begin, stream_it_t end) {
    std::stringstream str;
    RM_R inst;
    std::int16_t RHS;

    raw_deserialize<RM_R>(inst, begin, end);

    auto [ LHS, size ] = decode_MOV_RM_RI_body(inst, begin, end);
    if(inst.m_W == 1) {
        raw_deserialize<int16_t>(RHS, begin + size, end);
        size += 2;
    } else {
        RHS = *(begin + size);
        size += 1;
    }

    str << "MOV " << LHS << ", " << format_immediate(RHS, inst.m_W);
    return { str.str(), size };
}


decode_inst_t decode_MOV_I_R(stream_it_t begin, stream_it_t end) {
    std::stringstream str;
    I_R inst;

    raw_deserialize<I_R>(inst, begin, end);
    std::string LHS = decode_REG(inst.m_REG, inst.m_W);

    int16_t RHS;
    int size;
    if(inst.m_W == 1) {
        raw_deserialize<int16_t>(RHS, begin + 1, end);
        size = 3;
    } else {
        RHS = *(begin + 1);
        size = 2;
    }

    str << "MOV " << LHS << ", " << RHS;
    return { str.str(), size };
}


decode_inst_t decode_MOV_M_A(stream_it_t begin, stream_it_t end) {
    std::stringstream str;
    A inst;

    raw_deserialize<A>(inst, begin, end);
    std::string LHS = "AX";

    uint16_t RHS;
    std::string RHS_str;
    int size;
    if(inst.m_W == 1) {
        raw_deserialize<uint16_t>(RHS, begin + 1, end);
        size = 3;
    } else {
        RHS = *(begin + 1);
        size = 2;
    }

    if(!inst.m_D) {
        str << "MOV " << LHS << ", [" << RHS << ']';
    } else {
        str << "MOV " << '[' << RHS << ']' << ", " << LHS;
    }
    return { str.str(), size };
}

}
