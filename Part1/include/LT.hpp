#pragma once

#include "bit3mask.hpp"
#include "opcode.hpp"

#include <array>

namespace decode::opcode {
// OPCODE TABLE
//                         index_bits       index_addition
//                         |||              ||
constexpr auto MOV_RM_R = "100010xxxxxxxxxx_0"_bit3;
SET_BITMAP(MOV_RM_R, W w : 1; D d : 1; B b : 6; B rm : 3; REG reg : 3; MOD mod : 2;)

constexpr auto MOV_I_RM = "1100011xxx000xxx_0"_bit3;
SET_BITMAP(MOV_I_RM, W w : 1; B b : 7; B rm : 3; B b2 : 3; MOD mod : 2;)

constexpr auto MOV_I_R  = "1011xxxxxxxxxxxx_0"_bit3;
SET_BITMAP(MOV_I_R, REG reg : 3; W w : 1; B b : 4;)

constexpr auto MOV_M_A  = "101000xxxxxxxxxx_1"_bit3;
SET_BITMAP(MOV_M_A, W w : 1; D d : 1; B b : 6;)

constexpr auto ADD_RM_R = "000000xxxxxxxxxx_0"_bit3;
SET_BITMAP(ADD_RM_R, W w : 1; D d : 1; B b : 6; B rm : 3; REG reg : 3; MOD mod : 2;)

constexpr auto ADD_I_RM = "100000xxxx000xxx_1"_bit3;
SET_BITMAP(ADD_I_RM, W w : 1; S s : 1; B b : 6; B rm : 3; B b2 : 3; MOD mod : 2;)

constexpr auto ADD_I_A  = "0000010xxxxxxxxx_1"_bit3;
SET_BITMAP(ADD_I_A, W w : 1; B b : 7;)

constexpr auto SUB_RM_R = "001010xxxxxxxxxx_0"_bit3;
SET_BITMAP(SUB_RM_R, W w : 1; D d : 1; B b : 6; B rm : 3; REG reg : 3; MOD mod : 2;)

constexpr auto SUB_I_RM = "100000xxxx101xxx_2"_bit3;
SET_BITMAP(SUB_I_RM, W w : 1; S s : 1; B b : 6; B rm : 3; B b2 : 3; MOD mod : 2;)

constexpr auto SUB_I_A  = "0010110xxxxxxxxx_1"_bit3;
SET_BITMAP(SUB_I_A, W w : 1; B b : 7;)

constexpr auto CMP_RM_R = "001110xxxxxxxxxx_2"_bit3;
SET_BITMAP(CMP_RM_R, W w : 1; D d : 1; B b : 6; B rm : 3; REG reg : 3; MOD mod : 2;)

constexpr auto CMP_I_RM = "100000xxxx111xxx_3"_bit3;
SET_BITMAP(CMP_I_RM, W w : 1; S s : 1; B b : 6; B rm : 3; B b1 : 3; MOD mod : 2;)

constexpr auto CMP_I_A  = "0011110xxxxxxxxx_3"_bit3;
SET_BITMAP(CMP_I_A, W w : 1; S s : 1; B b : 6; B rm : 3; B b1 : 3; MOD mod : 2;)

constexpr auto JZ       = "01110100xxxxxxxx_0"_bit3;
constexpr auto JL       = "01111100xxxxxxxx_1"_bit3;
constexpr auto JLE      = "01111110xxxxxxxx_2"_bit3;
constexpr auto JB       = "01110010xxxxxxxx_3"_bit3;
constexpr auto JBE      = "01110110xxxxxxxx_4"_bit3;
constexpr auto JP       = "01111010xxxxxxxx_5"_bit3;
constexpr auto JO       = "01110000xxxxxxxx_6"_bit3;
constexpr auto JS       = "01111000xxxxxxxx_7"_bit3;
constexpr auto JNE      = "01110101xxxxxxxx_8"_bit3;
constexpr auto JNL      = "01111101xxxxxxxx_9"_bit3;
constexpr auto JG       = "01111111xxxxxxxx_10"_bit3;
constexpr auto JAE      = "01110011xxxxxxxx_11"_bit3;
constexpr auto JA       = "01110111xxxxxxxx_12"_bit3;
constexpr auto JPO      = "01111011xxxxxxxx_13"_bit3;
constexpr auto JNO      = "01110001xxxxxxxx_14"_bit3;
constexpr auto JNS      = "01111001xxxxxxxx_15"_bit3;

constexpr auto LOOP     = "11100010xxxxxxxx_0"_bit3;
constexpr auto LOOPZ    = "11100001xxxxxxxx_1"_bit3;
constexpr auto LOOPNZ   = "11100000xxxxxxxx_2"_bit3;
constexpr auto JCXZ     = "11100011xxxxxxxx_3"_bit3;

namespace details {

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
    SUB_I_A,

    CMP_RM_R,
    CMP_I_RM,
    CMP_I_A,

    JZ,
    JL,
    JLE,
    JB,
    JBE,
    JP,
    JO,
    JS,
    JNE,
    JNL,
    JG,
    JAE,
    JA,
    JPO,
    JNO,
    JNS,

    LOOP,
    LOOPZ,
    LOOPNZ,
    JCXZ,
};


template <typename Tuple>
constexpr bool validate_opcode_table(const Tuple& table) {
    bool seen[256]{}; // zero-initialized

    bool ok = true;
    std::apply([&](auto const&... entries) {
        (
            [&] {
                if (seen[entries.m_index]) {
                    ok = false;
                } else {
                    seen[entries.m_index] = true;
                }
            }(),
            ...
        );
    }, table);

    return ok;
}


template <typename Tuple>
std::array<bit3mask, 256> lookup_table_init(const Tuple& table) {
    std::array<bit3mask, 256> LT;

    std::apply([&](auto const&... entries) {
        (
            [&] {
                LT[entries.m_index] = entries.m_mask;
            }(),
            ...
        );
    }, table);

    return LT;
}


constexpr bool OPCODE_TABLE_VALID = validate_opcode_table(OPCODE_TABLE);
static_assert(OPCODE_TABLE_VALID == true);

}

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
                return i;
            }
        }

        return {};
    }

   public:
    std::array<bit3mask, 256> m_table = details::lookup_table_init(details::OPCODE_TABLE);
};


}

