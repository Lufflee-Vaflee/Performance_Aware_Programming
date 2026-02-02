#pragma once

#include <cstdint>

#include "op.hpp"

#include <array>

namespace state {

struct CPU {
    using RX = std::uint16_t;

    struct RHL {
        uint8_t RL;
        uint8_t RH;
    };

    union R {
        RHL rhl;
        RX  rx;
    };

    std::array<R, 8> regs;  //general purpose
    std::array<uint8_t, 65536> mem;
};



}

