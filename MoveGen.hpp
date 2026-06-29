//
// Created by Piotrek on 30.05.2026.
//

#ifndef BITBOARD_CHESS_MOVEGEN_HPP
#define BITBOARD_CHESS_MOVEGEN_HPP

#include <array>

#include "Gameboard.hpp"
#include "move.hpp"




namespace MoveGen {

    inline  std::array<BitBoard, 64>  KNIGHT_ATTACKS;
    MoveList get_all_moves(GameBoard board,Color color);

    MoveList generate_knight_moves(const GameBoard board,Color color);
    MoveList generate_king_moves(const GameBoard board,Color color);
    MoveList generate_pawn_moves(const GameBoard board,Color color);
    MoveList generate_sliding_moves(const GameBoard board, Color color);
};


#endif //BITBOARD_CHESS_MOVEGEN_HPP
