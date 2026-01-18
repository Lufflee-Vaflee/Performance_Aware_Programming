#pragma once

#include <cassert>
#include <array>
#include <tuple>

#include "bit3mask.hpp"

namespace OPCODE::details {

template <typename Tuple>
constexpr bool validate_OPCODE_TABLE(const Tuple& table) {
    bool seen[256]{}; // zero-initialized

    bool ok = true;
    std::apply([&](auto const&... entries) {
        (
            [&] {
                if (seen[entries.m_index]) {
                    ok = false;
                } else {
                    seen[entries.m_index] = true;
                }
            }(),
            ...
        );
    }, table);

    return ok;
}


template <typename Tuple>
std::array<bit3mask, 256> OPCODE_LOOKUP_TABLE_INIT(const Tuple& table) {
    std::array<bit3mask, 256> LT;

    std::apply([&](auto const&... entries) {
        (
            [&] {
                LT[entries.m_index] = entries.m_mask;
            }(),
            ...
        );
    }, table);

    return LT;
}

}
