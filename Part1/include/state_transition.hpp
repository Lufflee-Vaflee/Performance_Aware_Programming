#pragma once

#include <cstdint>
#include <array>
#include <iostream>

namespace state {

constexpr std::size_t max_mem_size = 1048576;
using mem_t = std::array<char, max_mem_size>;
using mem_it_t = mem_t::iterator;

class state {
    state() = default;

   public:

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
        // Bitfield definitions
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

        // Default constructor
        constexpr FREG() : 
            CF(0), b0(0), PF(0), b1(0), AF(0), b2(0), 
            ZF(0), SF(0), TF(0), IF(0), DF(0), OF(0), b3(0) {}

        // initializer_list constructor
        constexpr FREG(std::initializer_list<bool> list) : FREG() {
            auto it = list.begin();
            if (it != list.end()) CF = *it++;
            if (it != list.end()) b0 = *it++;
            if (it != list.end()) PF = *it++;
            if (it != list.end()) b1 = *it++;
            if (it != list.end()) AF = *it++;
            if (it != list.end()) b2 = *it++;
            if (it != list.end()) ZF = *it++;
            if (it != list.end()) SF = *it++;
            if (it != list.end()) TF = *it++;
            if (it != list.end()) IF = *it++;
            if (it != list.end()) DF = *it++;
            if (it != list.end()) OF = *it++;
            // b3 is 4 bits, would usually require a separate logic or cast
        }

        // Must be constexpr to use at compile-time
        // Note: bit_cast is constexpr since C++20
        constexpr operator uint16_t() const {
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
        static std::size_t count = 0;
        count++;
        std::cout << "dump count: " << count << '\n';
        std::cout << std::hex;
        std::cout << "AX: 0x" << regs[0].rx << '\n';
        std::cout << "BX: 0x" << regs[1].rx << '\n';
        std::cout << "CX: 0x" << regs[2].rx << '\n';
        std::cout << "DX: 0x" << regs[3].rx << '\n';

        std::cout << "SP: 0x" << regs[4].rx << '\n';
        std::cout << "BP: 0x" << regs[5].rx << '\n';
        std::cout << "SI: 0x" << regs[6].rx << '\n';
        std::cout << "DI: 0x" << regs[7].rx << '\n';

        std::cout << "IP: 0x" << IP << '\n';


        std::cout << "---------------------------------------\n";
    }
};

void cycle(mem_it_t begin, mem_it_t end, mem_it_t dump_end);

}

