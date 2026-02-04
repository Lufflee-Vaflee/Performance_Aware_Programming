#include "state_transition.hpp"
#include "decode.hpp"
#include <utility>

namespace state {


auto st = state::getInstance();

using post_d_t = std::pair<char*, code::W>;

void mov_dispatch(op::arg_t lhs, op::arg_t rhs) {
    struct visitor {
        post_d_t operator()(op::reg_arg_t reg) {
            using namespace code;
            switch(reg.reg) {
                case REG::AL_AX: return { reg.w ? reinterpret_cast<char*>(st.regs[0].rx) : reinterpret_cast<char*>(st.regs[0].rhl.rl), reg.w }; 
                case REG::CL_CX: return { reg.w ? reinterpret_cast<char*>(st.regs[2].rx) : reinterpret_cast<char*>(st.regs[2].rhl.rl), reg.w }; 
                case REG::DL_DX: return { reg.w ? reinterpret_cast<char*>(st.regs[3].rx) : reinterpret_cast<char*>(st.regs[3].rhl.rl), reg.w };
                case REG::BL_BX: return { reg.w ? reinterpret_cast<char*>(st.regs[1].rx) : reinterpret_cast<char*>(st.regs[1].rhl.rl), reg.w };
                case REG::AH_SP: return { reg.w ? reinterpret_cast<char*>(st.regs[4].rx) : reinterpret_cast<char*>(st.regs[0].rhl.rh), reg.w };
                case REG::CH_BP: return { reg.w ? reinterpret_cast<char*>(st.regs[5].rx) : reinterpret_cast<char*>(st.regs[2].rhl.rh), reg.w };
                case REG::DH_SI: return { reg.w ? reinterpret_cast<char*>(st.regs[6].rx) : reinterpret_cast<char*>(st.regs[3].rhl.rh), reg.w };
                case REG::BH_DI: return { reg.w ? reinterpret_cast<char*>(st.regs[7].rx) : reinterpret_cast<char*>(st.regs[1].rhl.rh), reg.w };
            }
        }
        post_d_t operator()(op::mem_arg_t mem) {
            return { reinterpret_cast<char*>(&st.mem[mem.mem]), mem.w };
        }
        post_d_t operator()(op::dis_mem_arg_t dis_mem) {
            using namespace code;
            switch(dis_mem.reg) {
                case DIS::BX_SI : return { &st.mem[st.regs[1].rx + st.regs[6].rx + dis_mem.dis], 0 };
                case DIS::BX_DI : return { &st.mem[st.regs[1].rx + st.regs[7].rx + dis_mem.dis], 0 };
                case DIS::BP_SI : return { &st.mem[st.regs[5].rx + st.regs[6].rx + dis_mem.dis], 0 };
                case DIS::BP_DI : return { &st.mem[st.regs[5].rx + st.regs[7].rx + dis_mem.dis], 0 };
                case DIS::SI    : return { &st.mem[st.regs[6].rx + dis_mem.dis];
                case DIS::DI    : return { &st.mem[st.regs[7].rx + dis_mem.dis];
                case DIS::DIRECT: assert(false); std::unreachable();
                case DIS::BX    : return &st.mem[st.regs[1].rx + dis_mem.dis];
            }
        }
        char* operator()(op::no_arg_t) {
            assert(false); std::unreachable();
        }
        char* operator()(op::)

    };
}

void cycle(mem_it_t begin, mem_it_t end) {
    while(true) { // TODO: change ending execution method
        mem_it_t cur = begin + st.IP;
        auto op = decode::decode(cur, end);
        st.IP = cur - begin;

        switch(op.id) {
        using namespace op;
        case MOV:
            
        case ADD:
        case SUB:
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

        }
    }

}

}

