// SlidingAttacks.hpp
#pragma once
#include "BitBoard.hpp"
#include "types.hpp"

namespace MoveGen {
    void init_sliders();

    BitBoard get_rook_attacks(Square sq, BitBoard occupied);
    BitBoard get_bishop_attacks(Square sq, BitBoard occupied);
    BitBoard get_queen_attacks(Square sq, BitBoard occupied);
}