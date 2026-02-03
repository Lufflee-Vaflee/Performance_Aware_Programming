#pragma once

#include "op.hpp"
#include "state_transition.hpp"

namespace decode {

using data_stream_t = state::mem_t;
using stream_it_t = state::mem_it_t;
op::decoded decode(stream_it_t& begin, stream_it_t end);

}
