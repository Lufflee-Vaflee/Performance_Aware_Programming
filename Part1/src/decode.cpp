#include "decode.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>

#include "LT.hpp"

#include "mov.hpp"

namespace decode {

std::string to_lower(std::string const& data) {
    std::string result;
    result.resize(data.size());
     std::transform(data.cbegin(), data.cend(), result.begin(),
        [](unsigned char c){ return std::tolower(c); });
     return result;
}


instr_stream_t load_input_stream(std::fstream& stream) {
    std::vector<char> result;
    stream.seekg(0, std::ios::end);
    auto fsize = stream.tellg();
    stream.seekg(0, std::ios::beg);
    result.resize(fsize);

    stream.read(static_cast<char*>(result.data()), fsize);
    return result;
}

OPCODE::ID peek_opcode_ident(decode::stream_it_t begin, decode::stream_it_t end) {
    char byte1 = *begin;
    char byte2 = 0;
    if(end - begin > 1) {
        byte2 = *(begin + 1);
    }

    uint16_t opcode = 0;
    char* dest = reinterpret_cast<char*>(&opcode);
    dest[0] = byte2;
    dest[1] = byte1;

    auto lt = OPCODE::LT::getInstance();
    return lt[opcode];
}

std::string JT(OPCODE::ID op_id, stream_it_t& begin, stream_it_t end) {
    using namespace OPCODE;
    int advance = 0;
    std::string str;
    switch (op_id) {
        case MOV_RM_R:
            std::tie(str, advance) = MOV::decode_MOV_RM_R(begin, end); break;
        case MOV_I_RM:
            std::tie(str, advance) = MOV::decode_MOV_I_RM(begin, end); break;
        default:
            throw "unimplemented";
    }

    begin += advance;
    return str;
}


std::string decode(instr_stream_t& instr_stream) {
    for(auto it = instr_stream.begin(); it != instr_stream.end();) {
        auto op_id = peek_opcode_ident(it, instr_stream.end());
        std::cout << to_lower(JT(op_id, it, instr_stream.end())) << '\n';
    }

    return {};
}


}

