//
// Created by Piotrek on 24.05.2026.
//

#ifndef BITBOARD_CHESS_GAMEBOARD_HPP
#define BITBOARD_CHESS_GAMEBOARD_HPP
#include "BitBoard.hpp"
#include "types.hpp"

struct GameBoard {
    private:
    static constexpr int ColorCount = 2;
    static constexpr int  PieceCount = 6;
    BitBoard all_bitboards[ColorCount][PieceCount] = {};
    public:

    inline BitBoard get_bitboard(Color color, PieceType piece) const {
        return all_bitboards[static_cast<int>(color)][static_cast<int>(piece)];
    }

    inline BitBoard get_bitboard_color(Color color) const{
        auto board = BitBoard(0ull);
        for (int i = 0; i < PieceCount; ++i) {
            board|=all_bitboards[static_cast<int>(color)][i];
        }
        return board;
    }

    inline BitBoard get_bitboard_all() const{
        auto board = BitBoard(0);
        for (const auto & color_bit_boards : all_bitboards) {
            for (const auto & piece : color_bit_boards) {
                board|=piece;
            }
        }
        return board;
    }

    void set_bitboard(Color color,PieceType piece, BitBoard board) {
      all_bitboards[static_cast<int>(color)][static_cast<int>(piece)] = board;
    }
};


#endif //BITBOARD_CHESS_GAMEBOARD_HPP
