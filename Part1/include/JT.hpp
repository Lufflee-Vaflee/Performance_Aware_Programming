#pragma once

#include "decode.hpp"

namespace decode {

namespace MOV {
    decode_inst_t RM_R(stream_it_t begin, stream_it_t end);
    decode_inst_t I_RM(stream_it_t begin, stream_it_t end);
    decode_inst_t DI_R(stream_it_t begin, stream_it_t end);
    decode_inst_t M_A(stream_it_t begin, stream_it_t end);
}

namespace ADD {
    decode_inst_t RM_R(stream_it_t begin, stream_it_t end);
    decode_inst_t I_RM(stream_it_t begin, stream_it_t end);
    decode_inst_t I_A(stream_it_t begin, stream_it_t end);
}

namespace SUB {
    decode_inst_t RM_R(stream_it_t begin, stream_it_t end);
    decode_inst_t I_RM(stream_it_t begin, stream_it_t end);
    decode_inst_t I_A(stream_it_t begin, stream_it_t end);
}

namespace CMP {
    decode_inst_t RM_R(stream_it_t begin, stream_it_t end);
    decode_inst_t I_RM(stream_it_t begin, stream_it_t end);
    decode_inst_t I_A(stream_it_t begin, stream_it_t end);
}

}
