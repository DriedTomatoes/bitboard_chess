//
// Created by Piotrek on 30.05.2026.
//

#include "MoveGen.hpp"








constexpr inline BitBoard compute_king_attacks(Square sq) {
    const int rank = static_cast<int>(sq) / 8;
    const int file = static_cast<int>(sq) % 8;

    constexpr int rank_diff[8] = {-1, -1, -1,  0, 0,  1, 1, 1};
    constexpr int file_diff[8] = {-1,  0,  1, -1, 1, -1, 0, 1};

    uint64_t attacks = 0ull;
    for (int r = 0; r < 8; ++r) {
        int target_rank = rank + rank_diff[r];
        int target_file = file + file_diff[r];
        if (target_file >= 0 && target_file < 8 && target_rank >= 0 && target_rank < 8) {
            attacks |= (1ull << (target_rank * 8 + target_file));
        }
    }
    return BitBoard(attacks);
}

inline constexpr std::array<BitBoard, 64> build_king_attacks() {
    std::array<BitBoard, 64> king_attacks{};
    for (int s = 0; s < 64; ++s) {
        king_attacks[s] = compute_king_attacks(Square(s));
    }
    return king_attacks;
}

inline constexpr std::array<BitBoard, 64> KING_ATTACKS = build_king_attacks();

constexpr uint64_t FILE_A = 0x0101010101010101ull;
constexpr uint64_t FILE_H = 0x8080808080808080ull;

constexpr inline BitBoard compute_pawn_attacks(Square sq, Color color) {
    uint64_t bit = 1ull << static_cast<int>(sq);
    uint64_t attacks = 0ull;

    if (color == Color::White) {
        if (bit & ~FILE_A) attacks |= (bit << 7);
        if (bit & ~FILE_H) attacks |= (bit << 9);
    } else {
        if (bit & ~FILE_H) attacks |= (bit >> 7);
        if (bit & ~FILE_A) attacks |= (bit >> 9);
    }
    return BitBoard(attacks);
}

inline constexpr std::array<std::array<BitBoard, 64>, 2> PAWN_ATTACKS = []() {
    std::array<std::array<BitBoard, 64>, 2> table{};
    for (int c = 0; c < 2; ++c) {
        for (int s = 0; s < 64; ++s) {
            table[c][s] = compute_pawn_attacks(Square(s), static_cast<Color>(c));
        }
    }
    return table;
}();




namespace MoveGen {
    MoveList get_all_moves(GameBoard board,Color color) {
        MoveList moves;
        for (auto m : generate_knight_moves(board, color) ) {
            moves.push_back(m);
        }
        for (auto k : generate_king_moves(board, color)) {
            moves.push_back(k);
        }
        for (auto p : generate_pawn_moves(board, color)) {
           moves.push_back(p);
        }
        MoveList sliders = generate_sliding_moves(board, color);
        for (int i = 0; i < sliders.count; ++i) moves.push_back(sliders.moves[i]);


       return moves;
    }

    MoveList generate_knight_moves(const GameBoard board,Color color) {
        MoveList knight_moves;

        BitBoard knights = board.get_bitboard(color,PieceType::Knight);
        BitBoard owned_squares = board.get_bitboard_color(color);

        for (Square from : knights) {

            BitBoard targets = KNIGHT_ATTACKS[static_cast<int>(from)] & ~owned_squares;


            for (Square to : targets) {
                knight_moves.push_back(Move{from,to});

            }

        }

        return knight_moves;
    }


        MoveList generate_king_moves(const GameBoard board, Color color) {
            MoveList king_moves;

            BitBoard king = board.get_bitboard(color, PieceType::King);
            BitBoard owned_squares = board.get_bitboard_color(color);

            for (Square from : king) {
                BitBoard targets = KING_ATTACKS[static_cast<int>(from)] & ~owned_squares;

                for (Square to : targets) {
                    king_moves.push_back(Move{from, to});
                }
            }
            return king_moves;
        }
    MoveList generate_pawn_moves(const GameBoard board, Color color) {
        MoveList pawn_moves;

        BitBoard pawns = board.get_bitboard(color, PieceType::Pawn);

        BitBoard own_pieces = board.get_bitboard_color(color);
        const Color enemy_color = (color == Color::White) ? Color::Black : Color::White;
        BitBoard enemy_pieces = board.get_bitboard_color(enemy_color);

        uint64_t all_occupied = own_pieces.board | enemy_pieces.board;

        const int direction = (color == Color::White) ? 1 : -1;
        const int start_rank = (color == Color::White) ? 1 : 6;

        for (Square from : pawns) {
            int from_idx = static_cast<int>(from);
            int rank = from_idx / 8;

            int one_step = from_idx + (direction * 8);

            if (!(all_occupied & (1ull << one_step))) {
                pawn_moves.push_back(Move{from, static_cast<Square>(one_step)});

                if (rank == start_rank) {
                    int two_steps = from_idx + (direction * 16);
                    if (!(all_occupied & (1ull << two_steps))) {
                        pawn_moves.push_back(Move{from, static_cast<Square>(two_steps)});
                    }
                }
            }

            int color_idx = static_cast<int>(color);
            BitBoard targets = PAWN_ATTACKS[color_idx][from_idx] & enemy_pieces;

            for (Square to : targets) {
                pawn_moves.push_back(Move{from, to});
            }
        }

        return pawn_moves;
    }


    MoveList generate_sliding_moves(const GameBoard board, Color color) {
        MoveList sliding_moves;

        BitBoard rooks = board.get_bitboard(color, PieceType::Rook);
        BitBoard bishops = board.get_bitboard(color, PieceType::Bishop);
        BitBoard queens = board.get_bitboard(color, PieceType::Queen);

        BitBoard own_pieces = board.get_bitboard_color(color);

        BitBoard all_occupied = BitBoard(board.get_bitboard_color(Color::White).board |
                                         board.get_bitboard_color(Color::Black).board);

        for (Square from : rooks) {
            BitBoard targets = get_rook_attacks(from, all_occupied) & ~own_pieces;
            for (Square to : targets) {
                sliding_moves.push_back(Move{from, to});
            }
        }

        for (Square from : bishops) {
            BitBoard targets = get_bishop_attacks(from, all_occupied) & ~own_pieces;
            for (Square to : targets) {
                sliding_moves.push_back(Move{from, to});
            }
        }

        for (Square from : queens) {
            BitBoard targets = get_queen_attacks(from, all_occupied) & ~own_pieces;
            for (Square to : targets) {
                sliding_moves.push_back(Move{from, to});
            }
        }

        return sliding_moves;
    }



    }

