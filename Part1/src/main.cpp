#include <cassert>
#include <iostream>
#include <fstream>
#include <cstring>

#include "lex.hpp"
#include "state_transition.hpp"

constexpr const char* head = "bits 16\n\n";
bool lex_mod = false;

void load_input_stream(std::fstream& stream) {
    stream.seekg(0, std::ios::end);
    auto fsize = static_cast<std::size_t>(stream.tellg());
    stream.seekg(0, std::ios::beg);
    //add few dummy bytes to ensure that 
    auto st = state::state::getInstance().get_mem();

    if(fsize > state::max_mem_size) {
        throw "shit";
    }

    stream.read(st.first, fsize);
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


    std::cout << head;
    try {
        lex::cycle();
    } catch(const char * str) {
        std::cout << str << '\n';
    } catch(...) {
        std::cout << "Caught unknown exception\n";
    }

    return 0;
}

