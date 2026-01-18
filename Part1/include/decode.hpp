#pragma once

#include <string>
#include <vector>
#include <cassert>

namespace decode {

using instr_stream_t = std::vector<char>;
using stream_it_t = std::vector<char>::iterator;

std::string decode(instr_stream_t&);
instr_stream_t load_input_stream(std::fstream& stream);

using decode_inst_t = std::pair<std::string, int>;

template<typename T>
//TODO add POD restriction
void raw_deserialize(T& dest, stream_it_t begin, stream_it_t end) {
    char* c_dest = reinterpret_cast<char*>(&dest);
    stream_it_t c_end = begin + sizeof(T);
    assert(end >= c_end);

    std::size_t i = 0;
    while(begin != c_end) {
        c_dest[i] = *begin;
        begin++;
        i++;
    }

    return;
}

}

