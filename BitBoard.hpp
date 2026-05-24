//
// Created by Piotrek on 22.05.2026.
//

#ifndef BITBOARD_CHESS_BITBOARD_HPP
#define BITBOARD_CHESS_BITBOARD_HPP

#include <bit>
#include <cstdint>
#include "types.hpp"

struct BitBoard {
    std::uint64_t board = 0;

    constexpr BitBoard(std::uint64_t board) : board(board) {}
    constexpr BitBoard() = default;

    BitBoard& operator|=(const BitBoard& rval) {
        board |= rval.board;
        return *this;
    }
    BitBoard& operator&=(const BitBoard& rval) {
        board &= rval.board;
        return *this;
    }
    BitBoard& operator^=(const BitBoard& rval) {
        board ^= rval.board;
        return *this;
    }
    bool operator==(const BitBoard& rval) const {
        return board == rval.board;
    }

    struct Iterator {
        uint64_t i_board;
        constexpr explicit Iterator(std::uint64_t x) : i_board(x) {}
        constexpr Square operator*() const { return static_cast<Square>(std::countr_zero(i_board)); }
        constexpr Iterator& operator++() {i_board &= i_board - 1; return *this;}
    };

    constexpr Iterator begin() const { return Iterator(board); }
    static constexpr Iterator end() { return Iterator(0ull); }
    };

inline std::ostream& operator<<(std::ostream& os, const BitBoard bb) {
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            if ((bb.board >> square) & 1ull) {
                os << "X";
            }
            else
                os << "O";
        }
        os << std::endl;

    }
    return os;
}

    
constexpr inline BitBoard operator&(BitBoard lval, BitBoard rval) { return {lval.board & rval.board}; }
constexpr inline BitBoard operator|(BitBoard lval, BitBoard rval) { return {lval.board | rval.board}; }
constexpr inline BitBoard operator^(BitBoard lval, BitBoard rval) { return {lval.board ^ rval.board}; }
constexpr inline BitBoard operator~(BitBoard bb) { return {~bb.board}; }



#endif //BITBOARD_CHESS_BITBOARD_HPP
