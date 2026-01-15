#include <iostream>
#include <cstdint>
#include <bit>
#include <sstream>
#include <utility>
#include <fstream>
#include <algorithm>


enum class OPCODE : uint8_t {
    MOVE_RM_R = 0b100010
};

//destination/source
using D = bool;

//Word/byte
using W = bool;

//Mode encoding
enum class MOD : uint8_t {
    MEM_NO_DISPLACMENT = 0b00,
    MEM_8_DISPLACMENT = 0b01,
    MEM_16_DISPLACMENT = 0b10,
    REGISTER = 0b11
};

enum class REG : uint8_t {
    AL_AX = 0b000,
    CL_CX = 0b001,
    DL_DX = 0b010,
    BL_BX = 0b011,
    AH_SP = 0b100,
    CH_BP = 0b101,
    DH_SI = 0b110,
    BH_DI = 0b111,
};

struct Instruction { 
    Instruction() = default;
    Instruction(Instruction const& other) = default;
    Instruction(std::uint16_t&& other) {
        *this = std::bit_cast<Instruction>(other);
    }

    W m_W : 1;
    D m_D : 1;
    OPCODE m_OPCODE : 6;

    REG m_RM : 3;
    REG m_REG : 3;
    MOD m_MOD : 2;

    operator std::uint16_t() {
        return std::bit_cast<std::uint16_t>(*this);
    }

    friend std::istream& operator>>(std::istream& is, Instruction& inst);
};

std::fstream& operator>>(std::fstream& is, Instruction& inst) {
    is.read(reinterpret_cast<char*>(&inst), 2);
    return is; // Return the stream for chaining and error checks
}

static_assert(sizeof(Instruction) == 2);

std::string decode_REG(REG const& reg, W const& w) noexcept {
    switch(reg) {
        case REG::AL_AX: return w ? "AX" : "AL";
        case REG::CL_CX: return w ? "CX" : "CL";
        case REG::DL_DX: return w ? "DX" : "DL";
        case REG::BL_BX: return w ? "BX" : "BL";
        case REG::AH_SP: return w ? "SP" : "AH";
        case REG::CH_BP: return w ? "BP" : "CH";
        case REG::DH_SI: return w ? "SI" : "DH";
        case REG::BH_DI: return w ? "DI" : "BH";
    }

    std::unreachable();
}

std::string decode_MOV(Instruction& inst) {
    std::stringstream str;
    str << "MOV ";
    std::string LHS = decode_REG(inst.m_REG, inst.m_W);
    std::string RHS = decode_REG(inst.m_RM, inst.m_W);
    if(!inst.m_D) std::swap(LHS, RHS);

    switch(inst.m_MOD) {
        case MOD::REGISTER:
            str << LHS << ", " << RHS;
            return str.str();
        default: throw "MOV mode unimplemented";
    }

    return {};
}

std::string decode(Instruction& inst) {
    switch(inst.m_OPCODE) {
        case OPCODE::MOVE_RM_R: return decode_MOV(inst);

        default: 
            std::cout << (int)inst.m_OPCODE;
            throw "OPCODE Unimplemented";
    }
}

std::string to_lower(std::string const& data) {
    std::string result;
    result.resize(data.size());
     std::transform(data.cbegin(), data.cend(), result.begin(),
        [](unsigned char c){ return std::tolower(c); });
     return result;
}

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

    try {
        Instruction inst;
        std::cout << head;
        while(binary >> inst) {
            std::cout << to_lower(decode(inst)) << '\n';
        }
    } catch(const char * str) {
        std::cout << str << '\n';
    } catch(...) {
        std::cout << "Caught unknown exception\n";
    }

    return 0;
}

