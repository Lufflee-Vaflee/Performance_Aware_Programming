#include "add.hpp"


namespace decode::ADD {

decode_inst_t decode_ADD_RM_R(stream_it_t begin, stream_it_t end) {
    std::stringstream str;
    RM_R inst;
    std::string LHS;


    raw_deserialize<RM_R>(inst, begin, end);
    LHS = decode_REG(inst.m_REG, inst.m_W);
    auto [ RHS, size ] = decode_DISPLACMENT(inst, begin, end);

    if(!inst.m_D) std::swap(LHS, RHS);
    str << "ADD " << LHS << ", " << RHS;
    return {str.str(), size + 2};
}

decode_inst_t decode_ADD_I_RM(stream_it_t begin, stream_it_t end) {
}

decode_inst_t decode_ADD_I_A(stream_it_t begin, stream_it_t end) {
}

}

