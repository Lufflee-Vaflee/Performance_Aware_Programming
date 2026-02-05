#include "state_transition.hpp"
#include "decode.hpp"
#include "op.hpp"
#include <cstring>
#include <utility>
#include <bit>

namespace state {

auto st = state::getInstance();

using post_d_t = std::pair<char*, code::W>;

post_d_t post_decode(op::arg_t op) {
    struct visitor {
        post_d_t operator()(op::reg_arg_t reg) {
            using namespace code;
            switch(reg.reg) {
                case REG::AL_AX: return { reg.w ? reinterpret_cast<char*>(&st.regs[0].rx) : reinterpret_cast<char*>(&st.regs[0].rhl.rl), reg.w }; 
                case REG::CL_CX: return { reg.w ? reinterpret_cast<char*>(&st.regs[2].rx) : reinterpret_cast<char*>(&st.regs[2].rhl.rl), reg.w }; 
                case REG::DL_DX: return { reg.w ? reinterpret_cast<char*>(&st.regs[3].rx) : reinterpret_cast<char*>(&st.regs[3].rhl.rl), reg.w };
                case REG::BL_BX: return { reg.w ? reinterpret_cast<char*>(&st.regs[1].rx) : reinterpret_cast<char*>(&st.regs[1].rhl.rl), reg.w };
                case REG::AH_SP: return { reg.w ? reinterpret_cast<char*>(&st.regs[4].rx) : reinterpret_cast<char*>(&st.regs[0].rhl.rh), reg.w };
                case REG::CH_BP: return { reg.w ? reinterpret_cast<char*>(&st.regs[5].rx) : reinterpret_cast<char*>(&st.regs[2].rhl.rh), reg.w };
                case REG::DH_SI: return { reg.w ? reinterpret_cast<char*>(&st.regs[6].rx) : reinterpret_cast<char*>(&st.regs[3].rhl.rh), reg.w };
                case REG::BH_DI: return { reg.w ? reinterpret_cast<char*>(&st.regs[7].rx) : reinterpret_cast<char*>(&st.regs[1].rhl.rh), reg.w };
            }
        }
        post_d_t operator()(op::mem_arg_t mem) {
            return { reinterpret_cast<char*>(&st.mem[mem.mem]), mem.w };
        }
        post_d_t operator()(op::dis_mem_arg_t dis_mem) {
            using namespace code;
            switch(dis_mem.reg) {
                case DIS::BX_SI : return { &st.mem[st.regs[1].rx + st.regs[6].rx + dis_mem.dis], 1 };
                case DIS::BX_DI : return { &st.mem[st.regs[1].rx + st.regs[7].rx + dis_mem.dis], 1 };
                case DIS::BP_SI : return { &st.mem[st.regs[5].rx + st.regs[6].rx + dis_mem.dis], 1 };
                case DIS::BP_DI : return { &st.mem[st.regs[5].rx + st.regs[7].rx + dis_mem.dis], 1 };
                case DIS::SI    : return { &st.mem[st.regs[6].rx + dis_mem.dis], 1 };
                case DIS::DI    : return { &st.mem[st.regs[7].rx + dis_mem.dis], 1 };
                case DIS::DIRECT: assert(false); std::unreachable();
                case DIS::BX    : return{ &st.mem[st.regs[1].rx + dis_mem.dis], 1 }; 
            }
        }
        post_d_t operator()(code::DIS dis_mem) {
            using namespace code;
            switch(dis_mem) {
                case DIS::BX_SI : return { &st.mem[st.regs[1].rx + st.regs[6].rx], 1 };
                case DIS::BX_DI : return { &st.mem[st.regs[1].rx + st.regs[7].rx], 1 };
                case DIS::BP_SI : return { &st.mem[st.regs[5].rx + st.regs[6].rx], 1 };
                case DIS::BP_DI : return { &st.mem[st.regs[5].rx + st.regs[7].rx], 1 };
                case DIS::SI    : return { &st.mem[st.regs[6].rx], 1 };
                case DIS::DI    : return { &st.mem[st.regs[7].rx], 1 };
                case DIS::DIRECT: assert(false); std::unreachable();
                case DIS::BX    : return{ &st.mem[st.regs[1].rx], 1 }; 
            }
        }
        post_d_t operator()(op::no_arg_t) {
            return { nullptr, 0 };
        }
        post_d_t operator()(op::immediate_w_arg_t im) {
            return { reinterpret_cast<char*>(&im.im), im.w };
        }
        post_d_t operator()(op::label_arg_t l) {
            return { reinterpret_cast<char*>(&l), 0 };
        }
    };

    return std::visit(visitor{}, op);
}

void mov_dispatch(op::arg_t lhs, op::arg_t rhs) {
    auto dest = post_decode(lhs);
    auto source = post_decode(rhs);

    uint8_t size = 1 + static_cast<unsigned>(dest.second || source.second);

    std::memmove(dest.first, source.first, size);
}

void add_dispatch(op::arg_t lhs, op::arg_t rhs) {
    auto dest = post_decode(lhs);
    auto source = post_decode(rhs);

    uint8_t size = 1 + static_cast<unsigned>(dest.second || source.second);

    if(size == 1) {
        uint8_t d;
        uint8_t s;
        uint8_t r;
        decode::raw_deserialize<uint8_t>(s, source.first, source.first + 1);
        decode::raw_deserialize<uint8_t>(d, dest.first, dest.first + 1);
        r = d + s;
        decode::raw_serialize<uint8_t>(r, dest.first, dest.first + 1);
    } else {
        uint16_t d;
        uint16_t s;
        uint16_t r;
        decode::raw_deserialize<uint16_t>(s, source.first, source.first + 2);
        decode::raw_deserialize<uint16_t>(d, dest.first, dest.first + 2);
        r = d + s;
        decode::raw_serialize<uint16_t>(r, dest.first, dest.first + 2);
    }

    return;
}


void sub_dispatch(op::arg_t lhs, op::arg_t rhs) {
    auto dest = post_decode(lhs);
    auto source = post_decode(rhs);

    uint8_t size = 1 + static_cast<unsigned>(dest.second || source.second);

    if(size == 1) {
        uint8_t d;
        uint8_t s;
        uint8_t r;
        decode::raw_deserialize<uint8_t>(s, source.first, source.first + 1);
        decode::raw_deserialize<uint8_t>(d, dest.first, dest.first + 1);
        r = d - s;
        decode::raw_serialize<uint8_t>(r, dest.first, dest.first + 1);
    } else {
        uint16_t d;
        uint16_t s;
        uint16_t r;
        decode::raw_deserialize<uint16_t>(s, source.first, source.first + 2);
        decode::raw_deserialize<uint16_t>(d, dest.first, dest.first + 2);
        r = d - s;
        decode::raw_serialize<uint16_t>(r, dest.first, dest.first + 2);
    }

    return;
}

void set_arithmetic_freg(uint8_t r, uint8_t d, uint8_t s) {
    st.freg.OF = r < d && r < s;
    st.freg.SF = (r & 0b10000000) == 0b10000000;
    st.freg.PF = (std::countl_zero(r) % 2) + 1;
    st.freg.ZF = (r == 0);

}

void cycle(mem_it_t begin, mem_it_t end) {
    while((begin + st.IP) != end) { // TODO: change ending execution method
        mem_it_t cur = begin + st.IP;
        auto op = decode::decode(cur, end);
        st.IP = cur - begin;

        switch(op.id) {
        using namespace op;
        case MOV:
            mov_dispatch(op.LHS, op.RHS);
            break;
        case ADD:
            add_dispatch(op.LHS, op.RHS);
            break;
        case SUB:
            sub_dispatch(op.LHS, op.RHS);
            break;
        case CMP:
        case JZ :
        case JL :
        case JLE:
        case JB :
        case JBE:
        case JP :
        case JO :
        case JS :
        case JNE:
        case JNL:
        case JG :
        case JAE:
        case JA :
        case JPO:
        case JNO:
        case JNS:
        case LOOP:
        case LOOPZ:
        case LOOPNZ:
        case JCXZ:
        default:
            throw "ALARM";
        }

        st.dump(begin, end);
    }
}

}
