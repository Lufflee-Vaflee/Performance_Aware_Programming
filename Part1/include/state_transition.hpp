#pragma once

#include <cstdint>
#include <array>
#include <iostream>

namespace state {

constexpr std::size_t max_mem_size = 65536;
using mem_t = std::array<char, max_mem_size>;
using mem_it_t = mem_t::iterator;

class state {
   private:
    state() = default;

    using RX = std::uint16_t;

    struct RHL {
        uint8_t rl;
        uint8_t rh;
    };

    union R {
        RHL rhl;
        RX  rx;
    };

    struct FREG {
        bool        CF: 1;
        uint8_t     b0: 1;
        bool        PF: 1;
        uint8_t     b1: 1;
        bool        AF: 1;
        uint8_t     b2: 1;
        bool        ZF: 1;
        bool        SF: 1;
        bool        TF: 1;
        bool        IF: 1;
        bool        DF: 1;
        bool        OF: 1;
        uint8_t     b3: 4;

        operator uint16_t() {
            return std::bit_cast<uint16_t>(*this);
        }
    };

    static_assert(sizeof(FREG) == 2);

   public:
    static state& getInstance() {
        static state st;
        return st;
    }

    std::pair<mem_it_t, mem_it_t> get_mem() {
        return { mem.begin(), mem.end() };
    }

    RX IP = 0;
    FREG freg;

    std::array<R, 8> regs;  //general purpose
    std::array<RX, 4> segment_regs;
    mem_t mem;

    void dump(mem_it_t begin, mem_it_t end) {
        std::cout << "AX: " << regs[0].rx << '\n';
        std::cout << "BX: " << regs[1].rx << '\n';
        std::cout << "CX: " << regs[2].rx << '\n';
        std::cout << "DX: " << regs[3].rx << '\n';
        std

        std::cout << "---------------------------------------\n";
    }
};

void cycle(mem_it_t begin, mem_it_t end);

}

