#include <cassert>
#include <iostream>
#include <fstream>
#include <cstring>

#include "lex.hpp"
#include "state_transition.hpp"

constexpr const char* head = "bits 16\n\n";
bool lex_mod = false;
std::size_t size = 0;

void load_input_stream(std::fstream& stream) {
    stream.seekg(0, std::ios::end);
    size = static_cast<std::size_t>(stream.tellg());
    stream.seekg(0, std::ios::beg);
    //add few dummy bytes to ensure that 
    auto st = state::state::getInstance().get_mem();

    if(size > state::max_mem_size) {
        throw "shit";
    }

    stream.read(st.first, size);
    return;
}

int main(int argc, char** argv) {
    if(argc < 2 || argc >= 4) {
        std::cout << "Usage: 8086sim [path_to_encoded_binary] (optional)-lex\n";
    }

    std::fstream binary(argv[1], std::ios_base::in | std::ios_base::binary);
    if(!binary.is_open()) {
        std::cout << "File didnt found\n";
        return 0;
    }

    if(argc == 3 && strncmp(argv[2], "-lex", 4)) {
        lex_mod = true;
    }

    load_input_stream(binary);

    auto begin = state::state::getInstance().get_mem().first;
    std::cout << head;
    try {
        state::cycle(begin, begin + size);
    } catch(const char * str) {
        std::cout << str << '\n';
    } catch(...) {
        std::cout << "Caught unknown exception\n";
    }

    return 0;
}

