#include "lex.hpp"

#include <sstream>
#include <utility>

namespace lex {


//std::string to_lower(std::string const& data) {
//    std::string result;
//    result.resize(data.size());
//     std::transform(data.cbegin(), data.cend(), result.begin(),
//        [](unsigned char c){ return std::tolower(c); });
//     return result;
//}


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

void cycle(decode::stream_it_t begin, decode::stream_it_t end) {
    return;
}

}

