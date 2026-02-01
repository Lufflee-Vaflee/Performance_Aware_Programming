#include "decode.hpp"

#include <cstring>
#include <utility>

#include "LT.hpp"

namespace decode {

//note: unsafe, doesnt check for end
opcode::ID peek_opcode_ident(decode::stream_it_t begin) {
    uint16_t opcode = 0;
    char* dest = reinterpret_cast<char*>(&opcode);
    dest[0] = *(begin + 1);
    dest[1] = *begin;

    auto lt = opcode::LT::getInstance();
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

inline std::pair<opcode::arg_t, int> decode_RM(opcode::unpacked_bitmap bitmap, stream_it_t begin, stream_it_t end) {
    using namespace opcode;
    dis_mem_arg_t ARG_dis;
    reg_arg_t ARG_reg;

    assert(bitmap.mod.has_value());
    assert(bitmap.rm.has_value());
    switch (*bitmap.mod) {
        case MOD::REGISTER:
            assert(bitmap.w.has_value());
            ARG_reg = { REG(*(bitmap.rm)), (*bitmap.w) };
            return { ARG_reg , 0 };
        case MOD::MEM_NO_DISPLACMENT:
            if((DIS(*(bitmap.rm))) == DIS::DIRECT) {
                mem_arg_t mem;
                raw_deserialize<uint16_t>(mem.mem, begin, end);
                mem.w = true;
                return { mem , 2 };
            }

            return { DIS(*(bitmap.rm)), 0 };
        case MOD::MEM_8_DISPLACMENT:
            ARG_dis = { *(begin), DIS(*(bitmap.rm)) };
            return { ARG_dis, 1 };
        case MOD::MEM_16_DISPLACMENT:
            raw_deserialize<int16_t>(ARG_dis.displacment, begin, end);
            ARG_dis.reg = *(bitmap.rm);
            return { ARG_dis, 2 };
    }

    std::unreachable();
}

inline std::pair<opcode::arg_t, int> decode_immediate(opcode::unpacked_bitmap bitmap, stream_it_t begin, stream_it_t end) {
    using namespace opcode;
    immediate_w_arg_t arg;

    assert(bitmap.w.has_value());
    auto w = *(bitmap.w);
    auto s = bitmap.s.has_value() ? *(bitmap.s) : 0;

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

inline std::pair<opcode::arg_t, int> decode_mem(opcode::unpacked_bitmap bitmap, stream_it_t begin, stream_it_t end) {
    using namespace opcode;
    mem_arg_t arg;

    assert(bitmap.w.has_value());
    auto w = *(bitmap.w);
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

inline std::pair<opcode::arg_t, int> decode_reg(opcode::unpacked_bitmap bitmap, stream_it_t begin, stream_it_t end) {
    assert(bitmap.reg.has_value());
    assert(bitmap.w.has_value());
    opcode::reg_arg_t ARG_reg { *(bitmap.reg), *(bitmap.w) };
    return { ARG_reg, 0 };
}

inline std::pair<opcode::arg_t, int> decode_AX(opcode::unpacked_bitmap bitmap, stream_it_t begin, stream_it_t end) {
    assert(bitmap.w.has_value());
    opcode::reg_arg_t ARG_reg { opcode::REG::AL_AX, *(bitmap.w) };
    return { ARG_reg, 0 };
}

template <typename F>
concept Delegator =
    std::invocable<
        F,
        opcode::unpacked_bitmap,
        stream_it_t,
        stream_it_t
    > &&
    std::same_as<
        std::invoke_result_t<
        F,
        opcode::unpacked_bitmap,
        stream_it_t,
        stream_it_t
        >,
        std::pair<opcode::arg_t, int>
    >;

template <opcode::ID id, auto D1, auto D2>
requires(
    Delegator<decltype(D1)> &&
    Delegator<decltype(D2)>)
decode_inst_t generalized_decode(stream_it_t begin, stream_it_t end) {
    using bitmask_t = opcode::get_bitmap<id>::t;
    bitmask_t packed;
    raw_deserialize<bitmask_t>(packed, begin, end);

    opcode::unpacked_bitmap unpacked = unpack_bitmap<bitmask_t>(packed);
    begin += sizeof(bitmask_t);
    decode_arg_t LHS_d = D1(unpacked, begin, end); 
    begin += LHS_d.second;
    decode_arg_t RHS_d = D2(unpacked, begin, end);

    int size = sizeof(bitmask_t) + LHS_d.second + RHS_d.second;
    if constexpr (opcode::details::has_d<bitmask_t>) {
        if (!(*unpacked.d)) {
            std::swap(LHS_d.first, RHS_d.first);
        }
    }

    return { { id, LHS_d.first, RHS_d.first}, size };
}

template<opcode::ID c_id = 256>
inline decode_inst_t decode_conditional_j(stream_it_t begin, stream_it_t end) {
    using namespace opcode;

    ID id = c_id;
    if constexpr (c_id == 256) {
        constexpr ID arr[16] = {
            JO ,
            JNO,
            JB ,
            JAE,
            JZ ,
            JNE,
            JBE,
            JA ,
            JS ,
            JNS,
            JP ,
            JPO,
            JL ,
            JNL,
            JLE,
            JG 
        };

        uint8_t code = *begin & 0b00001111;
        id = arr[code];
    }

    label_arg_t im = *(begin + 1);
    return { { id, im, no_arg_t{} }, 2};
}

opcode::decoded decode(stream_it_t& begin, stream_it_t end) {
    using namespace opcode;
    int advance = 0;
    opcode::decoded op;

    auto op_id = peek_opcode_ident(begin);
    switch (op_id) {
        case MOV_RM_R:
            std::tie(op, advance) = generalized_decode<MOV_RM_R, decode_reg, decode_RM>     (begin, end); break;
        case MOV_I_RM:
            std::tie(op, advance) = generalized_decode<MOV_I_RM, decode_RM,  decode_immediate>  (begin, end); break;
        case MOV_I_R:
            std::tie(op, advance) = generalized_decode<MOV_I_R,  decode_reg, decode_immediate>  (begin, end); break;
        case MOV_M_A:
            std::tie(op, advance) = generalized_decode<MOV_M_A,  decode_mem, decode_AX>   (begin, end); break;
        case ADD_RM_R:
            std::tie(op, advance) = generalized_decode<ADD_RM_R, decode_reg, decode_RM>     (begin, end); break;
        case ADD_I_RM:
            std::tie(op, advance) = generalized_decode<ADD_I_RM, decode_RM,  decode_immediate>  (begin, end); break;
        case ADD_I_A:
            std::tie(op, advance) = generalized_decode<ADD_I_A,  decode_AX,  decode_immediate>  (begin, end); break;
        case SUB_RM_R:
            std::tie(op, advance) = generalized_decode<SUB_RM_R, decode_reg, decode_RM>     (begin, end); break;
        case SUB_I_RM:
            std::tie(op, advance) = generalized_decode<SUB_I_RM, decode_RM,  decode_immediate>  (begin, end); break;
        case SUB_I_A:
            std::tie(op, advance) = generalized_decode<SUB_I_A,  decode_AX,  decode_immediate>  (begin, end); break;
        case CMP_RM_R:
            std::tie(op, advance) = generalized_decode<CMP_RM_R, decode_reg, decode_RM>     (begin, end); break;
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

