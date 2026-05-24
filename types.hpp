//
// Created by Piotrek on 24.05.2026.
//

#ifndef BITBOARD_CHESS_TYPES_HPP
#define BITBOARD_CHESS_TYPES_HPP
#include <cstdint>
#define COL(row) \
    A##row, B##row, C##row,D##row,E##row,F##row,G##row,H##row

enum Square : int8_t {
    COL(1), COL(2), COL(3), COL(4), COL(5), COL(6),COL(7), COL(8)
//A1, B1, C1... H8
};
#undef COL

enum class Color  {
    White = 0, Black = 1
};

enum class PieceType {
    Pawn = 0, Knight = 1, Bishop = 2, Rook = 3, Queen = 4, King = 5
};


#endif //BITBOARD_CHESS_TYPES_HPP
