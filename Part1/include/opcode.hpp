#pragma once

#include <cassert>
#include <variant>
#include <type_traits>
#include <limits>

#include "bit3mask.hpp"

namespace opcode {

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
    uint8_t rw;

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

static_assert(sizeof(bool) == 1);

namespace details {
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

    template<typename T>
    struct optionale_selector {
        //unimplemented
        static_assert(false);
        using U = void;
    };

    template<> 
    struct optionale_selector<bool> {
        using U = uint8_t;
        static constexpr U v = 3;
    };

    template<typename T>
    requires(std::is_enum_v<T>)
    struct optionale_selector<T> {
        using U = std::underlying_type_t<T>;
        static constexpr U v = std::numeric_limits<U>::max();
    };

}

template<typename T, typename U = details::optionale_selector<T>::U, U nulloption = details::optionale_selector<T>::v>
struct optionale_base {
    static_assert(sizeof(T) == sizeof(U));
    static_assert(alignof(T) == alignof(U));

    optionale_base() {
        value.u = nulloption;
    }

    union data {
        T t;
        U u;
    };

    bool has_value() {
        auto u = std::bit_cast<U>(value);
        if(u == nulloption) {
            return false;
        }
        
        value.t = std::bit_cast<T>(u);
        return true;
    }

    void reset() {
        value.u = nulloption;
    }

    T& operator*() {
        return value.t;
    }

   private:
    data value;
};

template<typename T>
struct optionale : optionale_base<T> {};

static constexpr uint8_t test = 255;

// zero-initialized
struct unpacked_bitmap {
    optionale<D> d;
    optionale<W> w;
    optionale<S> s;
    optionale<V> v;
    optionale<Z> z;
    optionale<REG> reg;
    optionale<MOD> mod;
    optionale_base<RM, uint8_t, test>  rm;
};

static_assert(sizeof(unpacked_bitmap) == 8);

template<typename packed>
unpacked_bitmap unpack_bitmap(packed data) {
    using namespace details;
    unpacked_bitmap result;

    if constexpr (has_d<packed>) {
        *(result.d)   = data.d;
    }
    if constexpr (has_w<packed>) {
        *(result.w)   = data.w;
    }
    if constexpr (has_s<packed>) {
        *(result.s)   = data.s;
    }
    if constexpr (has_v<packed>) {
        *(result.v)   = data.v;
    }
    if constexpr (has_z<packed>) {
        *(result.z)   = data.z;
    }
    if constexpr (has_mod<packed>) {
        *(result.mod) = data.mod;
    }
    if constexpr (has_reg<packed>) {
        *(result.reg) = data.reg;
    }
    if constexpr (has_rw<packed>) {
        *(result.rm)  = data.rw;
    }

    return result;
}

//also any that could be decribed as "just a num, including addresses"
using immediate = int16_t;
using direct_addr = uint16_t;

struct displacment_mem {
    int16_t displacment;
    DIS reg;
};

using noarg = uint8_t;

static_assert(sizeof(displacment_mem) == 4);

using arg_t = std::variant<REG, DIS, immediate, displacment_mem, direct_addr, noarg>;
static_assert(sizeof(arg_t) == 6);

struct decoded {
    ID id;
    arg_t LHS;
    arg_t RHS;
};

static_assert(sizeof(decoded) == 16);

}

