#pragma once

#include "code.hpp"
#include <variant>

namespace op {

enum logical {
    MOV,
    ADD,
    SUB,
    CMP,
    JO,
    JNO,
    JB,
    JAE,
    JZ,
    JNE,
    JBE,
    JA,
    JS,
    JNS,
    JP,
    JPO,
    JL,
    JNL,
    JLE,
    JG,
    LOOP,
    LOOPZ,
    LOOPNZ,
    JCXZ,
};

typedef uint8_t label_arg_t;

struct immediate_w_arg_t {
    int16_t im;
    code::W w;
};

struct mem_arg_t {
    uint16_t mem;
    code::W w;
};

struct dis_mem_arg_t {
    int16_t displacment;
    code::DIS reg;
};

struct reg_arg_t {
    code::REG reg;
    code::W   w;
};

typedef uint8_t no_arg_t;

static_assert(sizeof(dis_mem_arg_t) == 4);

using arg_t = std::variant<reg_arg_t, code::DIS, label_arg_t, immediate_w_arg_t, dis_mem_arg_t, mem_arg_t, no_arg_t>;
static_assert(sizeof(arg_t) == 6);

struct decoded {
    logical id;
    arg_t LHS;
    arg_t RHS;
};

static_assert(sizeof(decoded) == 16);


}
