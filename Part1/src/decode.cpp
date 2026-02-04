#include "decode.hpp"

#include <cstring>
#include <utility>

#include "LT.hpp"
#include "op.hpp"

namespace decode {

using decode_inst_t = std::pair<op::decoded, int>;
using decode_arg_t =  std::pair<op::arg_t, int>;

//note: unsafe, doesnt check for end
code::ID peek_opcode_ident(decode::stream_it_t begin) {
    uint16_t opcode = 0;
    char* dest = reinterpret_cast<char*>(&opcode);
    dest[0] = *(begin + 1);
    dest[1] = *begin;

    auto lt = code::LT::getInstance();
    return lt[opcode];
}

template<typename T>
//TODO add POD restriction
void raw_deserialize(T& dest, stream_it_t begin, stream_it_t end) {
    char* c_dest = reinterpret_cast<char*>(&dest);
    stream_it_t c_end = begin + sizeof(T);
    assert(end >= c_end);

    std::size_t i = 0;
    while(begin != c_end) {
        c_dest[i] = *begin;
        begin++;
        i++;
    }

    return;
}

template<typename bitmap_t>
std::pair<op::arg_t, int> decode_RM(bitmap_t bitmap, stream_it_t begin, stream_it_t end) {
    using namespace code;
    op::dis_mem_arg_t ARG_dis;
    op::reg_arg_t ARG_reg;

    static_assert(has_mod<bitmap_t>);
    static_assert(has_rm<bitmap_t>);
    switch (bitmap.mod) {
        case MOD::REGISTER:
            static_assert(has_w<bitmap_t>);
            ARG_reg = { REG(bitmap.rm), bitmap.w };
            return { ARG_reg , 0 };
        case MOD::MEM_NO_DISPLACMENT:
            if((DIS(bitmap.rm)) == DIS::DIRECT) {
                op::mem_arg_t mem;
                raw_deserialize<uint16_t>(mem.mem, begin, end);
                mem.w = true;
                return { mem , 2 };
            }

            return { DIS(bitmap.rm), 0 };
        case MOD::MEM_8_DISPLACMENT:
            ARG_dis = { *(begin), DIS(bitmap.rm) };
            return { ARG_dis, 1 };
        case MOD::MEM_16_DISPLACMENT:
            raw_deserialize<int16_t>(ARG_dis.dis, begin, end);
            ARG_dis.reg = DIS(bitmap.rm);
            return { ARG_dis, 2 };
    }

    std::unreachable();
}

template<typename bitmap_t>
std::pair<op::arg_t, int> decode_immediate(bitmap_t bitmap, stream_it_t begin, stream_it_t end) {
    using namespace code;
    op::immediate_w_arg_t arg;

    static_assert(has_w<bitmap_t>);
    auto w = bitmap.w;
    S s;
    if constexpr(has_s<bitmap_t>) {
        s = bitmap.s;
    } else {
        s = 0;
    }

    int S_W = s;
    S_W <<= 1;
    S_W += w;

    switch(S_W) {
    case 0:
        arg = { *(begin), w };
        return { arg, 1 };
    case 1:
        raw_deserialize<int16_t>(arg.im, begin, end);
        arg = { arg.im, w };
        return { arg , 2 };
    case 3:
        arg = { *(begin), w };
        if(arg.im >= 0) {
            return { arg, 1 };
        } else {
            arg.im |= 0xFF00;
            return { arg, 1 };
        }
    }

    std::unreachable();
}

template<typename bitmap_t>
std::pair<op::arg_t, int> decode_mem(bitmap_t bitmap, stream_it_t begin, stream_it_t end) {
    using namespace code;
    op::mem_arg_t arg;

    static_assert(has_w<bitmap_t>);
    auto w = bitmap.w;
    int size = 0;

    if(w) {
        raw_deserialize<uint16_t>(arg.mem, begin, end);
        size = 2;
    } else {
        arg.mem = *begin;
        size = 1;
    }

    arg.w = w;
    return { arg, size };
}

template<typename bitmap_t>
std::pair<op::arg_t, int> decode_reg(bitmap_t bitmap, stream_it_t begin, stream_it_t end) {
    static_assert(code::has_reg<bitmap_t>);
    static_assert(code::has_w<bitmap_t>);
    op::reg_arg_t ARG_reg { bitmap.reg, bitmap.w };
    return { ARG_reg, 0 };
}

template<typename bitmap_t>
std::pair<op::arg_t, int> decode_AX(bitmap_t bitmap, stream_it_t begin, stream_it_t end) {
    static_assert(code::has_w<bitmap_t>);
    op::reg_arg_t ARG_reg { code::REG::AL_AX, bitmap.w };
    return { ARG_reg, 0 };
}

inline op::logical code_to_op(code::ID id) {
    switch(id) {
    case code::MOV_RM_R:
    case code::MOV_I_RM:
    case code::MOV_I_R:
    case code::MOV_M_A:
        return op::MOV;
    case code::ADD_RM_R:
    case code::ADD_I_RM:
    case code::ADD_I_A:
        return op::ADD;
    case code::SUB_RM_R:
    case code::SUB_I_RM:
    case code::SUB_I_A:
        return op::SUB;
    case code::CMP_RM_R:
    case code::CMP_I_RM:
    case code::CMP_I_A:
        return op::CMP;
    case code::JZ :    return op::JZ;
    case code::JL :    return op::JL;
    case code::JLE:    return op::JLE;
    case code::JB :    return op::JB;
    case code::JBE:    return op::JBE;
    case code::JP :    return op::JP;
    case code::JO :    return op::JO;
    case code::JS :    return op::JS;
    case code::JNE:    return op::JNE;
    case code::JNL:    return op::JNL;
    case code::JG :    return op::JG;
    case code::JAE:    return op::JAE;
    case code::JA :    return op::JA;
    case code::JPO:    return op::JPO;
    case code::JNO:    return op::JNO;
    case code::JNS:    return op::JNS;
    case code::LOOP :  return op::LOOP;
    case code::LOOPZ:  return op::LOOPZ;
    case code::LOOPNZ: return op::LOOPNZ;
    case code::JCXZ:   return op::JCXZ;
    default:
        throw "ALARM";
    }
}

template<code::ID id>
using arg_delegator_t = std::pair<op::arg_t, int>(typename code::get_bitmap<id>::t, stream_it_t, stream_it_t);

template <code::ID id, arg_delegator_t<id> D1, arg_delegator_t<id> D2>
decode_inst_t generalized_decode(stream_it_t begin, stream_it_t end) {
    using bitmask_t = code::get_bitmap<id>::t;
    bitmask_t bitmap;
    raw_deserialize<bitmask_t>(bitmap, begin, end);

    begin += sizeof(bitmask_t);
    decode_arg_t LHS_d = D1(bitmap, begin, end); 
    begin += LHS_d.second;
    decode_arg_t RHS_d = D2(bitmap, begin, end);

    int size = sizeof(bitmask_t) + LHS_d.second + RHS_d.second;
    if constexpr (code::has_d<bitmask_t>) {
        if (!bitmap.d) {
            std::swap(LHS_d.first, RHS_d.first);
        }
    }

    
    return { { code_to_op(id), LHS_d.first, RHS_d.first }, size };
}

template<code::ID c_id = 256>
decode_inst_t decode_conditional_j(stream_it_t begin, stream_it_t end) {
    using namespace code;

    ID id = c_id;
    if constexpr (c_id == 256) {
        id = *begin & 0b11101111;
    }

    op::label_arg_t im = *(begin + 1);
    return { { code_to_op(id), im, op::no_arg_t{} }, 2};
}

op::decoded decode(stream_it_t& begin, stream_it_t end) {
    using namespace code;
    int advance = 0;
    op::decoded op;

    auto op_id = peek_opcode_ident(begin);
    switch (op_id) {
        case MOV_RM_R:
            std::tie(op, advance) = generalized_decode<MOV_RM_R, decode_reg, decode_RM>         (begin, end); break;
        case MOV_I_RM:
            std::tie(op, advance) = generalized_decode<MOV_I_RM, decode_RM,  decode_immediate>  (begin, end); break;
        case MOV_I_R:
            std::tie(op, advance) = generalized_decode<MOV_I_R,  decode_reg, decode_immediate>  (begin, end); break;
        case MOV_M_A:
            std::tie(op, advance) = generalized_decode<MOV_M_A,  decode_mem, decode_AX>         (begin, end); break;
        case ADD_RM_R:
            std::tie(op, advance) = generalized_decode<ADD_RM_R, decode_reg, decode_RM>         (begin, end); break;
        case ADD_I_RM:
            std::tie(op, advance) = generalized_decode<ADD_I_RM, decode_RM,  decode_immediate>  (begin, end); break;
        case ADD_I_A:
            std::tie(op, advance) = generalized_decode<ADD_I_A,  decode_AX,  decode_immediate>  (begin, end); break;
        case SUB_RM_R:
            std::tie(op, advance) = generalized_decode<SUB_RM_R, decode_reg, decode_RM>         (begin, end); break;
        case SUB_I_RM:
            std::tie(op, advance) = generalized_decode<SUB_I_RM, decode_RM,  decode_immediate>  (begin, end); break;
        case SUB_I_A:
            std::tie(op, advance) = generalized_decode<SUB_I_A,  decode_AX,  decode_immediate>  (begin, end); break;
        case CMP_RM_R:
            std::tie(op, advance) = generalized_decode<CMP_RM_R, decode_reg, decode_RM>         (begin, end); break;
        case CMP_I_RM:
            std::tie(op, advance) = generalized_decode<CMP_I_RM, decode_RM,  decode_immediate>  (begin, end); break;
        case CMP_I_A:
            std::tie(op, advance) = generalized_decode<CMP_I_A,  decode_AX,  decode_immediate>  (begin, end); break;
        case J:
            std::tie(op, advance) = decode_conditional_j(begin, end); break;
        case LOOP:
            std::tie(op, advance) = decode_conditional_j<LOOP>(begin, end); break;
        case LOOPZ:
            std::tie(op, advance) = decode_conditional_j<LOOPZ>(begin, end); break;
        case LOOPNZ:
            std::tie(op, advance) = decode_conditional_j<LOOPNZ>(begin, end); break;
        case JCXZ:
            std::tie(op, advance) = decode_conditional_j<JCXZ>(begin, end); break;
        default:
            throw "unimplemented";
    }

    begin += advance;
    return op;
}

}

