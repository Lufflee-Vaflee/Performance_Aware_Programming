#include "lex.hpp"
#include "op.hpp"
#include "decode.hpp"

#include <algorithm>
#include <sstream>
#include <utility>
#include <unordered_map>
#include <optional>
#include <iostream>
#include <vector>

namespace lex {

using namespace decode;

class label_gen {
   public:
    label_gen() = default;

    std::optional<std::size_t> check_for_label(stream_it_t abs) const {
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

    void reset(stream_it_t begin) {
        m_begin = begin;
        m_label_inc = 1;
        m_table.clear();
    }

   private:
    std::unordered_map<std::size_t, std::size_t> m_table;
    stream_it_t m_begin;
    std::size_t m_label_inc = 1;
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


inline std::string format_immediate(op::immediate_w_arg_t im) {
    std::stringstream result;
    result << (im.w ? "word " : "byte ") << im.im;
    return result.str();
}

inline std::string lex_REG(op::reg_arg_t reg) noexcept {
    using namespace code;
    switch(reg.reg) {
        case REG::AL_AX: return reg.w ? "AX" : "AL";
        case REG::CL_CX: return reg.w ? "CX" : "CL";
        case REG::DL_DX: return reg.w ? "DX" : "DL";
        case REG::BL_BX: return reg.w ? "BX" : "BL";
        case REG::AH_SP: return reg.w ? "SP" : "AH";
        case REG::CH_BP: return reg.w ? "BP" : "CH";
        case REG::DH_SI: return reg.w ? "SI" : "DH";
        case REG::BH_DI: return reg.w ? "DI" : "BH";
    }

    std::unreachable();
}

inline std::string lex_DIS(code::DIS dis) noexcept {
    using namespace code;
    switch(dis) {
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


inline std::string lex_DIS(op::dis_mem_arg_t im) noexcept {
    std::stringstream str;
    str << lex_DIS(im.reg) << format_displacment(im.dis);
    return str.str();
}

inline std::string format_mem(std::string mem) {
    std::stringstream str;
    str << '[' << mem << ']';
    return str.str();
}

inline std::string format_mem(op::mem_arg_t direct) {
    std::stringstream str;
    str << (direct.w ? "word" : "byte") << " [" << direct.mem << ']';
    return str.str();
}

//TODO: could be better
static label_gen gen;
static stream_it_t current_ip_position;

inline std::string lex_label(op::label_arg_t l) {
    auto label = gen.check_for_label(current_ip_position + l);
    assert(label.has_value());
    return std::string("label") + std::to_string(*label);
}

std::string arg_lex(op::arg_t arg) {
    using namespace code;

    struct visitor {
        std::string operator()(op::reg_arg_t reg) const { return lex_REG(reg); }
        std::string operator()(DIS dis) const { return format_mem(lex_DIS(dis)); }
        std::string operator()(op::dis_mem_arg_t mem) const { return format_mem(lex_DIS(mem)); }
        std::string operator()(op::label_arg_t l) const { return lex_label(l); }
        std::string operator()(op::immediate_w_arg_t im) const { return format_immediate(im); }
        std::string operator()(op::mem_arg_t direct) const { return format_mem(direct); }
        std::string operator()(op::no_arg_t no_arg) const { return ""; }
    };

    return std::visit(visitor{}, arg);
}

std::string opcode_lex(op::logical id) {
    using namespace op;
    switch(id) {
    case MOV:
        return "MOV";
    case ADD:
        return "ADD";
    case SUB:
        return "SUB";
    case CMP:
        return "CMP";
    case JZ :    return "JZ";
    case JL :    return "JL";
    case JLE:    return "JLE";
    case JB :    return "JB";
    case JBE:    return "JBE";
    case JP :    return "JP";
    case JO :    return "JO";
    case JS :    return "JS";
    case JNE:    return "JNE";
    case JNL:    return "JNL";
    case JG :    return "JG";
    case JAE:    return "JAE";
    case JA :    return "JA";
    case JPO:    return "JPO";
    case JNO:    return "JNO";
    case JNS:    return "JNS";
    case LOOP :  return "LOOP";
    case LOOPZ:  return "LOOPZ";
    case LOOPNZ: return "LOOPNZ";
    case JCXZ:   return "JCXZ";
    default:
        throw "ALARM";
    }
}

inline bool is_conditional_jmp(op::logical id) {
    using namespace op;
    return (id > JO && id < JCXZ);
}

void cycle() {
    auto [begin, end] = state::state::getInstance().get_mem();
    gen.reset(begin);

    std::vector<std::pair<op::decoded, decode::stream_it_t>> instructions;
    for(auto it = begin; it != end;) {
        auto save = it;
        auto decoded = decode::decode(it, end);
        if(is_conditional_jmp(decoded.id)) {
            gen.reg_label(save + std::get<2>(decoded.LHS));
        }

        instructions.emplace_back(decoded, save);
    }

    for(auto it = instructions.begin(); it != instructions.end(); ++it) {
        current_ip_position = (*it).second;
        auto label_num = gen.check_for_label(current_ip_position);

        std::string rhs = arg_lex(it->first.RHS);
        std::cout << opcode_lex(it->first.id) << " " << arg_lex(it->first.LHS);
        if(rhs.size() != 0) {
            std::cout << ", " << arg_lex(it->first.RHS);
        }
        std::cout << '\n';

        if(label_num.has_value()) {
            std::cout << "label" << *label_num << ":\n";
        }

    }

    return;
}


}

