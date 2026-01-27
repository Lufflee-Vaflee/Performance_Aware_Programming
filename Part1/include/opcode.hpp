#pragma once

#include <cassert>
#include <variant>

#include "bit3mask.hpp"

namespace decode::opcode {

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

struct RW {
    uint8_t rw = 0;

    operator DIS() {
        return static_cast<DIS>(rw);
    }

    operator REG() {
        return static_cast<REG>(rw);
    }
};

template<ID id>
struct get_bitmap {
    static_assert(false);
    using t = void;
};

#define SET_BITMAP(OP, TYPE)        \
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
concept has_rw =    requires(T t) { t.rw; };

// zero-initialized
struct unpacked_bitmap {
    D d = 0;
    W w = 0;
    S s = 0;
    V v = 0;
    Z z = 0;
    MOD mod = MOD::MEM_NO_DISPLACMENT;
    REG reg = REG::AL_AX;
    RW  rw;
};

static_assert(sizeof(unpacked_bitmap) == 8);

template<ID id>
unpacked_bitmap unpack_bitmap(uint16_t data) {
    using bitmap_t = get_bitmap<id>::bitmap;
    unpacked_bitmap result;
    bitmap_t bitmap = std::bit_cast<bitmap_t>(data);

    if constexpr (has_d<bitmap_t>) {
        result.d   = bitmap.d;
    }
    if constexpr (has_w<bitmap_t>) {
        result.w   = bitmap.w;
    }
    if constexpr (has_s<bitmap_t>) {
        result.s   = bitmap.s;
    }
    if constexpr (has_v<bitmap_t>) {
        result.v   = bitmap.v;
    }
    if constexpr (has_z<bitmap_t>) {
        result.z   = bitmap.z;
    }
    if constexpr (has_mod<bitmap_t>) {
        result.mod = bitmap.mod;
    }
    if constexpr (has_reg<bitmap_t>) {
        result.reg = bitmap.reg;
    }
    if constexpr (has_rw<bitmap_t>) {
        result.rw  = bitmap.rw;
    }

    return result;
}

//also any that could be decribed as "just a num"
using immediate = int16_t;

//direct addr also
struct displacment_mem {
    uint16_t displacment;
    DIS reg;
};

static_assert(sizeof(displacment_mem) == 4);

using arg_t = std::variant<REG, immediate, displacment_mem>;
static_assert(sizeof(arg_t) == 6);

struct decoded {
    ID id;
    arg_t LHS;
    arg_t RHS;
};

static_assert(sizeof(decoded) == 16);

}

