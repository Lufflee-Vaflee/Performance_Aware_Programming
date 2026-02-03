#include "state_transition.hpp"
#include "decode.hpp"

#include <tuple>

namespace state {

std::pair<mem_it_t, > 


void cycle(mem_it_t begin, mem_it_t end) {
    auto st = state::getInstance();

    while(true) { // TODO: change ending execution method
        mem_it_t cur = begin + st.IP;
        auto op = decode::decode(cur, end);
        st.IP = cur - begin;

        
    }

}

}

