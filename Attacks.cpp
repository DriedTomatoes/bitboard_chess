//
// Created by Piotrek on 29.06.2026.
#include "Attacks.hpp"

namespace Attacks {
    static BitBoard KnightAttacks[64];
    static BitBoard KingAttacks[64];
    static BitBoard WhitePawnAttacks[64];
    static BitBoard BlackPawnAttacks[64];

    constexpr uint64_t NOT_A_FILE  = 0xFEFEFEFEFEFEFEFEull; // Wszystko oprócz kolumny A
    constexpr uint64_t NOT_H_FILE  = 0x7F7F7F7F7F7F7F7Full; // Wszystko oprócz kolumny H
    constexpr uint64_t NOT_AB_FILE = 0xFCFCFCFCFCFCFCFCull; // Wszystko oprócz kolumn A i B
    constexpr uint64_t NOT_GH_FILE = 0x3F3F3F3F3F3F3F3Full; // Wszystko oprócz kolumn G i H

    void init() {
        for (int sq = 0; sq < 64; ++sq) {
            const uint64_t bit = 1ull << sq;
            uint64_t knight_mask = 0ull;
            uint64_t king_mask = 0ull;

            // Przesunięcia 17 15 10 6 to ruchy skoczka (L)
            knight_mask |= (bit << 17) & NOT_A_FILE;
            knight_mask |= (bit << 10) & NOT_AB_FILE;
            knight_mask |= (bit >>  6) & NOT_AB_FILE;
            knight_mask |= (bit >> 15) & NOT_A_FILE;
            knight_mask |= (bit << 15) & NOT_H_FILE;
            knight_mask |= (bit <<  6) & NOT_GH_FILE;
            knight_mask |= (bit >> 10) & NOT_GH_FILE;
            knight_mask |= (bit >> 17) & NOT_H_FILE;

            KnightAttacks[sq] = BitBoard(knight_mask);

            // --- ATAKI KRÓLA ---
            // Król rusza się o 1 pole w każdym z 8 kierunków (8 w pionie, 1 w poziomie, 7/9 na ukos)
            king_mask |= (bit << 8);                           // Góra
            king_mask |= (bit >> 8);                           // Dół
            king_mask |= (bit << 1) & NOT_A_FILE;              // Prawo
            king_mask |= (bit >> 1) & NOT_H_FILE;              // Lewo
            king_mask |= (bit << 9) & NOT_A_FILE;              // Góra-Prawo
            king_mask |= (bit << 7) & NOT_H_FILE;              // Góra-Lewo
            king_mask |= (bit >> 7) & NOT_A_FILE;              // Dół-Prawo
            king_mask |= (bit >> 9) & NOT_H_FILE;              // Dół-Lewo

            KingAttacks[sq] = BitBoard(king_mask);


            uint64_t white_pawn_mask = 0ull;
            // Białe biją o  8 więc en passant 7 i 9
            white_pawn_mask |= (bit & NOT_A_FILE) << 7; // Bicie w górę-lewo
            white_pawn_mask |= (bit & NOT_H_FILE) << 9; // Bicie w górę-prawo
            WhitePawnAttacks[sq] = BitBoard(white_pawn_mask);

            uint64_t black_pawn_mask = 0ull;
            // Czarne  -8, więc en passany  -9 , -7
            black_pawn_mask |= (bit & NOT_A_FILE) >> 9; // Bicie w dół-lewo
            black_pawn_mask |= (bit & NOT_H_FILE) >> 7; // Bicie w dół-prawo
            BlackPawnAttacks[sq] = BitBoard(black_pawn_mask);
        }
    }

    BitBoard get_knight_attacks(Square sq) {
        return KnightAttacks[static_cast<int>(sq)];
    }

    BitBoard get_king_attacks(Square sq) {
        return KingAttacks[static_cast<int>(sq)];
    }

    BitBoard get_pawn_attacks(Color color,Square sq) {
        if (color == Color::White) {
            return WhitePawnAttacks[sq];
        }
        else {
            return BlackPawnAttacks[sq];
        }
    }
}