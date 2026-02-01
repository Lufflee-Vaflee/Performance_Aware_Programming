#pragma once

#include <cassert>
#include <variant>

#include "bit3mask.hpp"

namespace code {

enum class MOD : uint8_t {
    MEM_NO_DISPLACMENT  = 0b00,
    MEM_8_DISPLACMENT   = 0b01,
    MEM_16_DISPLACMENT  = 0b10,
    REGISTER            = 0b11
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

enum class DIS : uint8_t {
    BX_SI   = 0b000,
    BX_DI   = 0b001,
    BP_SI   = 0b010,
    BP_DI   = 0b011,
    SI      = 0b100,
    DI      = 0b101,
    DIRECT  = 0b110,
    BX      = 0b111,
};

struct RM {
    uint8_t rm;

    operator DIS() {
        return static_cast<DIS>(rm);
    }

    operator REG() {
        return static_cast<REG>(rm);
    }
};

template<code::ID id>
struct get_bitmap {
    static_assert(false);
    using t = void;
};

#define SET_BITMAP(OP, TYPE)   \
template<>                          \
struct get_bitmap<OP> {             \
    using t = struct bitmap{TYPE};                 \
    static_assert(sizeof(bitmap) == 2 || sizeof(bitmap) == 1); \
};

//placeholder type
using B = uint8_t;

using D = bool;
using W = bool;
using S = bool;
using V = bool;
using Z = bool;

static_assert(sizeof(bool) == 1);

template <typename T>
concept has_d =     requires(T t) { t.d; };
template <typename T>
concept has_w =     requires(T t) { t.w; };
template <typename T>
concept has_s =     requires(T t) { t.s; };
template <typename T>
concept has_v =     requires(T t) { t.v; };
template <typename T>
concept has_z =     requires(T t) { t.z; };
template <typename T>
concept has_mod =   requires(T t) { t.mod; };
template <typename T>
concept has_reg =   requires(T t) { t.reg; };
template <typename T>
concept has_rm =    requires(T t) { t.rm; };
}

