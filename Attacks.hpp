//
// Created by Piotrek on 29.06.2026.
//

#ifndef BITBOARD_CHESS_ATTACKS_HPP
#define BITBOARD_CHESS_ATTACKS_HPP


#include "BitBoard.hpp"
#include "types.hpp"

namespace Attacks {


    // Inicjalizacja tablic (wywoływana raz na start)
    void init();

    // Dostęp do gotowych masek ataków
    BitBoard get_knight_attacks(Square sq);
    BitBoard get_king_attacks(Square sq);
    BitBoard get_pawn_attacks(Color color, Square sq);
}

#endif //BITBOARD_CHESS_ATTACKS_HPP
