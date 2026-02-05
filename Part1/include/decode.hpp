#pragma once

#include "op.hpp"
#include "state_transition.hpp"

namespace decode {

using data_stream_t = state::mem_t;
using stream_it_t = state::mem_it_t;
op::decoded decode(stream_it_t& begin, stream_it_t end);

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

template<typename T>
//TODO add POD restriction
void raw_serialize(T& source, stream_it_t begin, stream_it_t end) {
    char* c_source = reinterpret_cast<char*>(&source);
    stream_it_t c_end = begin + sizeof(T);
    assert(end >= c_end);

    std::size_t i = 0;
    while(begin != c_end) {
        *begin = c_source[i];
        begin++;
        i++;
    }

    return;
}

}
