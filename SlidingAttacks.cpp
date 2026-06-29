#include "SlidingAttacks.hpp"
#include "BitBoard.hpp"

bool slider_initialized = false;

namespace MoveGen {

    struct Magic {
        uint64_t mask;
        uint64_t magic;
        int shift;
        BitBoard* attacks;
    };

    Magic RookMagics[64];
    Magic BishopMagics[64];

    BitBoard RookTable[102400];
    BitBoard BishopTable[5248];

    constexpr int RookBits[64] = {
        12, 11, 11, 11, 11, 11, 11, 12,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        12, 11, 11, 11, 11, 11, 11, 12
    };

    constexpr int BishopBits[64] = {
        6, 5, 5, 5, 5, 5, 5, 6,
        5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5,
        6, 5, 5, 5, 5, 5, 5, 6
    };

    uint64_t get_rook_mask(const Square sq) {
        uint64_t mask = 0ull;
        const int r = sq / 8, f = sq % 8;

        for (int i = r + 1; i < 7; ++i) mask |= (1ull << (i * 8 + f));
        for (int i = r - 1; i > 0; --i) mask |= (1ull << (i * 8 + f));
        for (int i = f + 1; i < 7; ++i) mask |= (1ull << (r * 8 + i));
        for (int i = f - 1; i > 0; --i) mask |= (1ull << (r * 8 + i));

        return mask;
    }

    uint64_t get_bishop_mask(const Square sq) {
        uint64_t mask = 0ull;
        const int start_r = sq / 8, start_f = sq % 8;

        for (int r = start_r + 1, f = start_f + 1; r < 7 && f < 7; ++r, ++f) mask |= (1ull << (r * 8 + f));
        for (int r = start_r + 1, f = start_f - 1; r < 7 && f > 0; ++r, --f) mask |= (1ull << (r * 8 + f));
        for (int r = start_r - 1, f = start_f + 1; r > 0 && f < 7; --r, ++f) mask |= (1ull << (r * 8 + f));
        for (int r = start_r - 1, f = start_f - 1; r > 0 && f > 0; --r, --f) mask |= (1ull << (r * 8 + f));

        return mask;
    }

    uint64_t generate_occupancy(const int index, const uint64_t mask) {
        uint64_t occ = 0ull;
        int bit_count = 0;
        for (int i = 0; i < 64; i++) {
            if (mask & (1ull << i)) {
                if (index & (1 << bit_count)) {
                    occ |= (1ull << i);
                }
                bit_count++;
            }
        }
        return occ;
    }

    uint64_t build_rook_attacks_on_the_fly(int sq, uint64_t occ) {
        uint64_t attacks = 0ull;
        int r = sq / 8, f = sq % 8;
        for (int j = r + 1; j <= 7; j++) { attacks |= (1ull << (j * 8 + f)); if (occ & (1ull << (j * 8 + f))) break; }
        for (int j = r - 1; j >= 0; j--) { attacks |= (1ull << (j * 8 + f)); if (occ & (1ull << (j * 8 + f))) break; }
        for (int j = f + 1; j <= 7; j++) { attacks |= (1ull << (r * 8 + j)); if (occ & (1ull << (r * 8 + j))) break; }
        for (int j = f - 1; j >= 0; j--) { attacks |= (1ull << (r * 8 + j)); if (occ & (1ull << (r * 8 + j))) break; }
        return attacks;
    }

    uint64_t build_bishop_attacks_on_the_fly(int sq, uint64_t occ) {
        uint64_t attacks = 0ull;
        int r = sq / 8, f = sq % 8;
        for (int j = 1; r+j <= 7 && f+j <= 7; j++) { attacks |= (1ull << ((r+j)*8 + f+j)); if (occ & (1ull << ((r+j)*8 + f+j))) break; }
        for (int j = 1; r+j <= 7 && f-j >= 0; j++) { attacks |= (1ull << ((r+j)*8 + f-j)); if (occ & (1ull << ((r+j)*8 + f-j))) break; }
        for (int j = 1; r-j >= 0 && f+j <= 7; j++) { attacks |= (1ull << ((r-j)*8 + f+j)); if (occ & (1ull << ((r-j)*8 + f+j))) break; }
        for (int j = 1; r-j >= 0 && f-j >= 0; j++) { attacks |= (1ull << ((r-j)*8 + f-j)); if (occ & (1ull << ((r-j)*8 + f-j))) break; }
        return attacks;
    }

    uint64_t random_uint64() {
        static uint64_t seed = 1804289383ull;
        seed ^= seed >> 12;
        seed ^= seed << 25;
        seed ^= seed >> 27;
        return seed * 2685821657736338717ull;
    }

    uint64_t find_magic_number(const Square sq,const int bits,const bool is_bishop) {
        const uint64_t mask = is_bishop ? get_bishop_mask(sq) : get_rook_mask(sq);
        const int num_combinations = 1 << bits;

        uint64_t occupancies[4096];
        uint64_t attacks[4096];
        uint64_t used_attacks[4096];

        for (int i = 0; i < num_combinations; i++) {
            occupancies[i] = generate_occupancy(i, mask);
            attacks[i] = is_bishop ? build_bishop_attacks_on_the_fly(sq, occupancies[i])
                                   : build_rook_attacks_on_the_fly(sq, occupancies[i]);
        }

        for (int k = 0; k < 10000000; k++) {
            const uint64_t magic = random_uint64() & random_uint64() & random_uint64();
            if (((mask * magic) & 0xFF00000000000000ull) < 6) continue;

            for (int i = 0; i < num_combinations; i++) used_attacks[i] = 0ull;
            bool fail = false;
            const int shift = 64 - bits;

            for (int i = 0; i < num_combinations; i++) {
                int idx = (occupancies[i] * magic) >> shift;
                if (used_attacks[idx] == 0ull) {
                    used_attacks[idx] = attacks[i];
                } else if (used_attacks[idx] != attacks[i]) {
                    fail = true;
                    break;
                }
            }
            if (!fail) return magic;
        }
        return 0ull;
    }

    void init_sliders() {
        int rook_offset = 0;
        int bishop_offset = 0;

        for (int s = 0; s < 64; ++s) {
            const auto sq = static_cast<Square>(s);

            RookMagics[s].mask = get_rook_mask(sq);
            RookMagics[s].shift = 64 - RookBits[s];
            RookMagics[s].magic = find_magic_number(sq, RookBits[s], false);
            RookMagics[s].attacks = &RookTable[rook_offset];

            int rook_combinations = 1 << RookBits[s];
            for (int i = 0; i < rook_combinations; ++i) {
                uint64_t occ = generate_occupancy(i, RookMagics[s].mask);
                uint64_t att = build_rook_attacks_on_the_fly(s, occ);
                int idx = (occ * RookMagics[s].magic) >> RookMagics[s].shift;
                RookMagics[s].attacks[idx] = BitBoard(att);
            }
            rook_offset += rook_combinations;

            BishopMagics[s].mask = get_bishop_mask(sq);
            BishopMagics[s].shift = 64 - BishopBits[s];
            BishopMagics[s].magic = find_magic_number(sq, BishopBits[s], true);
            BishopMagics[s].attacks = &BishopTable[bishop_offset];

            int bishop_combinations = 1 << BishopBits[s];
            for (int i = 0; i < bishop_combinations; ++i) {
                uint64_t occ = generate_occupancy(i, BishopMagics[s].mask);
                uint64_t att = build_bishop_attacks_on_the_fly(s, occ);
                int idx = (occ * BishopMagics[s].magic) >> BishopMagics[s].shift;
                BishopMagics[s].attacks[idx] = BitBoard(att);
            }
            bishop_offset += bishop_combinations;
        }
        slider_initialized = true;
    }

    BitBoard get_rook_attacks(const Square sq, const BitBoard occupied) {
        if (!slider_initialized) init_sliders();

        uint64_t occ = occupied.board & RookMagics[sq].mask;
        occ *= RookMagics[sq].magic;
        occ >>= RookMagics[sq].shift;
        return RookMagics[sq].attacks[occ];
    }

    BitBoard get_bishop_attacks(const Square sq, const BitBoard occupied) {
        if (!slider_initialized) init_sliders();

        uint64_t occ = occupied.board & BishopMagics[sq].mask;
        occ *= BishopMagics[sq].magic;
        occ >>= BishopMagics[sq].shift;
        return BishopMagics[sq].attacks[occ];
    }

    BitBoard get_queen_attacks(Square sq, BitBoard occupied) {
        return BitBoard(get_rook_attacks(sq, occupied).board | get_bishop_attacks(sq, occupied).board);
    }
}