#pragma once

#include "opcode.hpp"

namespace OPCODE {

// OPCODE TABLE
//                         index_bits       index_addition
//                         |||              ||
constexpr auto MOV_RM_R = "100010xxxxxxxxxx_0"_bit3;
constexpr auto MOV_I_R  = "1011xxxxxxxxxxxx_0"_bit3;
constexpr auto MOV_M_A  = "101000xxxxxxxxxx_1"_bit3;
constexpr auto MOV_I_RM = "1100011xxx000xxx_0"_bit3;

constexpr auto ADD_RM_R = "000000xxxxxxxxxx_0"_bit3;
constexpr auto ADD_I_RM = "100000xxxx000xxx_1"_bit3;
constexpr auto ADD_I_A  = "0000010xxxxxxxxx_1"_bit3;

constexpr auto SUB_RM_R = "001010xxxxxxxxxx_0"_bit3;
constexpr auto SUB_I_RM = "100000xxxx101xxx_2"_bit3;
constexpr auto SUB_I_A  = "0010110xxxxxxxxx_1"_bit3;


constexpr auto OPCODE_TABLE = std::tuple {
    MOV_RM_R,
    MOV_I_R,
    MOV_M_A,
    MOV_I_RM,

    ADD_RM_R,
    ADD_I_RM,
    ADD_I_A,

    SUB_RM_R,
    SUB_I_RM,
    SUB_I_A 
};

constexpr bool OPCODE_TABLE_VALID = details::validate_OPCODE_TABLE(OPCODE_TABLE);
static_assert(OPCODE_TABLE_VALID == true);

class LT {
   private:
    LT() = default;

   public:
    static LT& getInstance() {
        static LT lt;
        return lt;
    }

    ID operator[](uint16_t opcode) {
        constexpr uint16_t mask = 0b1110000000000000;
        uint16_t index_group = opcode & mask;
        index_group >>= 8;

        for(uint8_t i = index_group; i < index_group + 32; ++i) {
            if(m_table[i].match(opcode)) {
                return {m_table[i], i};
            }
        }

        return {};
    }

   public:
    std::array<bit3mask, 256> m_table = details::OPCODE_LOOKUP_TABLE_INIT(OPCODE_TABLE);
};


}

