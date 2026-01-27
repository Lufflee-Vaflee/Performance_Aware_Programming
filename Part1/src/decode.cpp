#include "decode.hpp"

#include <algorithm>
#include <iostream>
#include <cstring>

#include "J_.hpp"
#include "LT.hpp"
#include "JT.hpp"

namespace decode {

//std::string to_lower(std::string const& data) {
//    std::string result;
//    result.resize(data.size());
//     std::transform(data.cbegin(), data.cend(), result.begin(),
//        [](unsigned char c){ return std::tolower(c); });
//     return result;
//}

opcode::ID peek_opcode_ident(decode::stream_it_t begin) {
    uint16_t opcode = 0;
    char* dest = reinterpret_cast<char*>(&opcode);
    dest[0] = (*begin + 1);
    dest[1] = *begin;

    auto lt = opcode::LT::getInstance();
    return lt[opcode];
}

static conditional_jmp_table jmp_table;
stream_it_t start;

std::string JT(opcode::ID op_id, stream_it_t& begin, stream_it_t end) {
    using namespace opcode;
    int advance = 0;
    std::string str;
    switch (op_id) {
        case MOV_RM_R:
            std::tie(str, advance) = MOV::RM_R(begin, end); break;
        case MOV_I_RM:
            std::tie(str, advance) = MOV::I_RM(begin, end); break;
        case MOV_I_R:
            std::tie(str, advance) = MOV::DI_R(begin, end); break;
        case MOV_M_A:
            std::tie(str, advance) = MOV::M_A(begin, end); break;
        case ADD_RM_R:
            std::tie(str, advance) = ADD::RM_R(begin, end); break;
        case ADD_I_RM:
            std::tie(str, advance) = ADD::I_RM(begin, end); break;
        case ADD_I_A:
            std::tie(str, advance) = ADD::I_A(begin, end); break;
        case SUB_RM_R:
            std::tie(str, advance) = SUB::RM_R(begin, end); break;
        case SUB_I_RM:
            std::tie(str, advance) = SUB::I_RM(begin, end); break;
        case SUB_I_A:
            std::tie(str, advance) = SUB::I_A(begin, end); break;
        case CMP_RM_R:
            std::tie(str, advance) = CMP::RM_R(begin, end); break;
        case CMP_I_RM:
            std::tie(str, advance) = CMP::I_RM(begin, end); break;
        case CMP_I_A:
            std::tie(str, advance) = CMP::I_A(begin, end); break;
        case JZ:
            std::tie(str, advance) = jmp_table.decode_conditional_jmp("JZ", start, begin, end); break;
        case JL:
            std::tie(str, advance) = jmp_table.decode_conditional_jmp("JL", start, begin, end); break;
        case JLE:
            std::tie(str, advance) = jmp_table.decode_conditional_jmp("JLE", start, begin, end); break;
        case JB:
            std::tie(str, advance) = jmp_table.decode_conditional_jmp("JB", start, begin, end); break;
        case JBE:
            std::tie(str, advance) = jmp_table.decode_conditional_jmp("JBE", start, begin, end); break;
        case JP:
            std::tie(str, advance) = jmp_table.decode_conditional_jmp("JP", start, begin, end); break;
        case JO:
            std::tie(str, advance) = jmp_table.decode_conditional_jmp("JO", start, begin, end); break;
        case JS:
            std::tie(str, advance) = jmp_table.decode_conditional_jmp("JS", start, begin, end); break;
        case JNE:
            std::tie(str, advance) = jmp_table.decode_conditional_jmp("JNE", start, begin, end); break;
        case JNL:
            std::tie(str, advance) = jmp_table.decode_conditional_jmp("JNL", start, begin, end); break;
        case JG:
            std::tie(str, advance) = jmp_table.decode_conditional_jmp("JG", start, begin, end); break;
        case JAE:
            std::tie(str, advance) = jmp_table.decode_conditional_jmp("JAE", start, begin, end); break;
        case JA:
            std::tie(str, advance) = jmp_table.decode_conditional_jmp("JA", start, begin, end); break;
        case JPO:
            std::tie(str, advance) = jmp_table.decode_conditional_jmp("JPO", start, begin, end); break;
        case JNO:
            std::tie(str, advance) = jmp_table.decode_conditional_jmp("JNO", start, begin, end); break;
        case JNS:
            std::tie(str, advance) = jmp_table.decode_conditional_jmp("JNS", start, begin, end); break;
        case LOOP:
            std::tie(str, advance) = jmp_table.decode_conditional_jmp("LOOP", start, begin, end); break;
        case LOOPZ:
            std::tie(str, advance) = jmp_table.decode_conditional_jmp("LOOPZ", start, begin, end); break;
        case LOOPNZ:
            std::tie(str, advance) = jmp_table.decode_conditional_jmp("LOOPNZ", start, begin, end); break;
        case JCXZ:
            std::tie(str, advance) = jmp_table.decode_conditional_jmp("JCXZ", start, begin, end); break;
        default:
            throw "unimplemented";
    }

    begin += advance;
    return str;
}

opcode_stream_t decode(data_stream_t& instr_stream) {
    opcode_stream_t result;
    result.reserve(instr_stream.size() / 6);


    //insert dummy byte to remove check at peek opcode
    auto size = instr_stream.size();
    instr_stream.emplace_back('0');

    start = instr_stream.begin();
    auto end = instr_stream.end();

    for(auto it = start; it != end;) {
        auto op_id = peek_opcode_ident(it);
        auto cur_pos = it - instr_stream.begin();


        std::string op = JT(op_id, it, instr_stream.end());
    }

//    std::size_t current_cs_position = 0;
//    for(auto it = ops.begin(); it != ops.end(); ++it) {
//        current_cs_position = (*it).second;
//        auto label_num = jmp_table[current_cs_position];
//
//        std::cout << (*it).first << '\n';
//        if(label_num != std::numeric_limits<std::size_t>::max()) {
//            std::cout << "label" << label_num << ":\n";
//        }
//
//    }

    return {};
}

}
    
