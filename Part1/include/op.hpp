#pragma once

#include "LT.hpp"

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

inline logical code_to_op(code::ID id) {
    switch(id) {
    case code::MOV_RM_R:
    case code::MOV_I_RM:
    case code::MOV_I_R:
    case code::MOV_M_A:
        return MOV;
    case code::ADD_RM_R:
    case code::ADD_I_RM:
    case code::ADD_I_A:
        return ADD;
    case code::SUB_RM_R:
    case code::SUB_I_RM:
    case code::SUB_I_A:
        return SUB;
    case code::CMP_RM_R:
    case code::CMP_I_RM:
    case code::CMP_I_A:
        return CMP;
    case code::JZ :    return JZ;
    case code::JL :    return JL;
    case code::JLE:    return JLE;
    case code::JB :    return JB;
    case code::JBE:    return JBE;
    case code::JP :    return JP;
    case code::JO :    return JO;
    case code::JS :    return JS;
    case code::JNE:    return JNE;
    case code::JNL:    return JNL;
    case code::JG :    return JG;
    case code::JAE:    return JAE;
    case code::JA :    return JA;
    case code::JPO:    return JPO;
    case code::JNO:    return JNO;
    case code::JNS:    return JNS;
    case code::LOOP :  return LOOP;
    case code::LOOPZ:  return LOOPZ;
    case code::LOOPNZ: return LOOPNZ;
    case code::JCXZ:   return JCXZ;
    default:
        throw "ALARM";
    }
}

typedef int16_t label_arg_t;

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
