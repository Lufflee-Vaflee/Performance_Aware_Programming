#include "lex.hpp"
#include "LT.hpp"

#include <algorithm>
#include <sstream>
#include <utility>
#include <unordered_map>
#include <optional>
#include <iostream>

namespace lex {

using namespace decode;

class label_gen {
   public:
    label_gen(stream_it_t begin) :
        m_begin(begin) {}

    std::optional<std::size_t> check_for_label(stream_it_t abs) {
        std::size_t pos = abs - m_begin;
        auto it = m_table.find(pos);
        if(it == m_table.end()) {
            return {};
        }

        return it->second;
    }

    void reg_label(stream_it_t abs) {
        std::size_t pos = abs - m_begin;
        m_table[pos] = m_label_inc;
        m_label_inc++;
    }

   private:
    std::unordered_map<std::size_t, std::size_t> m_table;
    std::size_t m_label_inc = 1;
    stream_it_t m_begin;
};


std::string to_lower(std::string const& data) {
    std::string result;
    result.resize(data.size());
     std::transform(data.cbegin(), data.cend(), result.begin(),
        [](unsigned char c){ return std::tolower(c); });
     return result;
}


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


std::stringstream& lex(opcode::decoded) {
    
}

inline bool is_conditional_jmp(opcode::ID id) {
    using namespace opcode;
    return ((id > JZ && id < JNS) || (id > LOOP && id < JCXZ));
}

void cycle(decode::stream_it_t begin, decode::stream_it_t end) {
    label_gen gen(begin);

    std::vector<std::pair<opcode::decoded, decode::stream_it_t>> instructions;
    for(auto it = begin; it != end;) {
        auto save = it;
        auto decoded = decode::decode(it, end);
        if(is_conditional_jmp(decoded.id)) {
            gen.reg_label(save + std::get<2>(decoded.LHS));
        }

        instructions.emplace_back(decoded, save);
    }

    for(auto it = instructions.begin(); it != instructions.end(); ++it) {
        auto current_cs_position = (*it).second;
        auto label_num = gen.check_for_label(current_cs_position);

        //std::cout << (*it).first << '\n';
        if(label_num != std::numeric_limits<std::size_t>::max()) {
            std::cout << "label" << *label_num << ":\n";
        }

    }

    return;
}


}

