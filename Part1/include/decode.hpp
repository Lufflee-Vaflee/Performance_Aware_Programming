#pragma once

#include <vector>

#include "opcode.hpp"

namespace decode {

using data_stream_t = std::vector<char>;
using stream_it_t = std::vector<char>::iterator;

using decode_inst_t = std::pair<code::decoded, int>;
using decode_arg_t =  std::pair<code::arg_t, int>;

code::decoded decode(stream_it_t& begin, stream_it_t end);

}
