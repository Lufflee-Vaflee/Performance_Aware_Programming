#include <cassert>
#include <iostream>
#include <fstream>
#include <cstring>

#include "decode.hpp"

constexpr const char* head = "bits 16\n\n";
bool lex_mod = false;
//temporary measure to remove uneescesarry bouandary checks, until state model is made
constexpr size_t safe_bytes = 6;

decode::data_stream_t load_input_stream(std::fstream& stream) {
    std::vector<char> result;
    stream.seekg(0, std::ios::end);
    auto fsize = static_cast<std::size_t>(stream.tellg());
    stream.seekg(0, std::ios::beg);
    //add few dummy bytes to ensure that 
    result.resize(fsize + safe_bytes);

    stream.read(static_cast<char*>(result.data()), fsize);
    return result;
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

    auto mem = load_input_stream(binary);

    std::cout << head;
    try {
        //decode::decode(instr_stream.begin(), instr_stream.end());
    } catch(const char * str) {
        std::cout << str << '\n';
    } catch(...) {
        std::cout << "Caught unknown exception\n";
    }

    return 0;
}

