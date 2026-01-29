#pragma once

#include <cstdint>
#include <cassert>
#include <string_view>
#include <stdexcept>

namespace decode::opcode {



using string_repr_t = char[16];

struct bit3mask {
   private:
    constexpr static bool valid_3bit(char c) {
        return c == '0' || c == '1' || c == 'x';
    }

    constexpr static std::uint16_t generate_xor_mask(string_repr_t const& mask) {
        uint16_t xor_mask = 0;
        for(int i = 0; i < 16; ++i) {
            xor_mask <<= 1;
            if(mask[i] == '1') {
                xor_mask += 1;
            }
        }

        return xor_mask;
    }

    constexpr static std::uint16_t generate_and_mask(string_repr_t const& mask) {
        uint16_t and_mask = 0;
        for(int i = 0; i < 16; ++i) {
            and_mask <<= 1;
            if(mask[i] != 'x') {
                and_mask += 1;
            }
        }

        return and_mask;
    }

    constexpr void initialize_from_literal(string_repr_t const& mask) {
        for(int i = 0; i < 16; ++i) {
            if(!valid_3bit(mask[i])) {
                throw "not valid 3bit";
            }
        }

        xor_mask = generate_xor_mask(mask);
        and_mask = generate_and_mask(mask);
    }


   public:
    bit3mask() = default;

    constexpr bit3mask(string_repr_t const& mask) {
        initialize_from_literal(mask);
    }

    constexpr bit3mask(bit3mask const& mask) = default;

    constexpr bit3mask& operator=(string_repr_t const& mask) {
        initialize_from_literal(mask);
        return *this;
    }

    constexpr bit3mask& operator=(bit3mask const& mask) = default;

    /*
    match illustration
    1010 1100 1110 0000 input

    1010 1xxx xxx0 00xx identifier
          ||| |||    ||
    1010 1000 0000 0000 xor mask
          ||| |||    ||
    0000 0100 1110 0000 xor result
          ||| |||    ||
    1111 1000 0001 1100 and mask

    0000 0000 0000 0000 final result
    */


    //1000 1001 1100 0011 input
    //1000 10xx xxxx xxxx_0
    //

    bool match(uint16_t in) {
        assert(!(xor_mask == 0 && and_mask == 0));

        in ^= xor_mask;
        in &= and_mask;
        return in == 0;
    }

    friend bool operator==(bit3mask const& lhs, bit3mask const& rhs) {
        return lhs.xor_mask == rhs.xor_mask && lhs.and_mask == rhs.and_mask;
    }

   private:
    uint16_t xor_mask = 0;
    uint16_t and_mask = 0;
};

using ID = uint32_t;

struct match_table_entry {
    constexpr match_table_entry() = default;
    constexpr match_table_entry(bit3mask mask, ID index) :
        m_mask(mask),
        m_index(index) {}

    bit3mask m_mask;
    ID m_index;

    constexpr operator ID() const {
        return m_index;
    }
};

constexpr int constexpr_stoi(std::string_view str) {
    std::size_t i = 0;

    int value = 0;
    for (; i < str.size(); ++i) {
        char c = str[i];
        if (c < '0' || c > '9') {
            throw std::invalid_argument("invalid digit");
        }
        value = value * 10 + (c - '0');
    }

    return value;
}

constexpr match_table_entry operator""_bit3(const char* mask, std::size_t len) {
    string_repr_t arr;
    for(std::size_t i = 0; i < 16; ++i)
        arr[i] = mask[i];

    if(mask[16] != '_') {
        throw "dont panic";
    }

    uint8_t index = 0;
    for(std::size_t i = 0; i < 7; ++i) {
        if(arr[i] == '1' && i < 3) {
            index += 1;
        }
        index <<= 1;
    }

    index += constexpr_stoi(&mask[17]);
    return match_table_entry { bit3mask(arr), index };
}

}

