#pragma once

#include "decode.hpp"

namespace decode::ADD {

static_assert(sizeof(RM_R_D) == 2);

decode_inst_t decode_ADD_RM_R(stream_it_t begin, stream_it_t end);
decode_inst_t decode_ADD_I_RM(stream_it_t begin, stream_it_t end);
decode_inst_t decode_ADD_I_A(stream_it_t begin, stream_it_t end);

}

