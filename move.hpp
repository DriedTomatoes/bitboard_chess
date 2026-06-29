//
// Created by Piotrek on 23.06.2026.
//


#ifndef BITBOARD_CHESS_MOVE_HPP
#define BITBOARD_CHESS_MOVE_HPP


#include <array>


#include "types.hpp"

struct Move {
    Square from;
    Square to;

    PieceType promotion = PieceType::None; // None oznacza brak promocji
    bool is_en_passant = false;
    bool is_castling = false;
};

struct MoveList {
    static constexpr int max_moves = 256; //218 is max number of LEGAL moves in a position
    std::array<Move, max_moves> moves;
    int count = 0;

    inline void push_back(Move m) {
        moves[count] = m;
        count++;
    }

    inline Move* begin() {
        return moves.data();
    }
    inline Move* end() {
        return moves.data() + count;
    }


};
#endif //BITBOARD_CHESS_MOVE_HPP
