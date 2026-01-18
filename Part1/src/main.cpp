#include <cassert>
#include <iostream>
#include <fstream>
#include <cstring>

#include "decode.hpp"

constexpr const char* head = "bits 16\n\n";

int main(int argc, char** argv) {
    if(argc != 2) {
        std::cout << "Usage: 8086sim [path_to_encoded_binary]\n";
    }

    std::fstream binary(argv[1], std::ios_base::in | std::ios_base::binary);
    if(!binary.is_open()) {
        std::cout << "File didnt found\n";
        return 0;
    }

    auto instr_stream = decode::load_input_stream(binary);

    std::cout << head;
    try {
        decode::decode(instr_stream);
    } catch(const char * str) {
        std::cout << str << '\n';
    } catch(...) {
        std::cout << "Caught unknown exception\n";
    }

    return 0;
}

