//
// Created by Piotrek on 24.05.2026.
//

#ifndef BITBOARD_CHESS_GAMEBOARD_HPP
#define BITBOARD_CHESS_GAMEBOARD_HPP
#include "Attacks.hpp"
#include "BitBoard.hpp"
#include "types.hpp"
#include "move.hpp"
#include "SlidingAttacks.hpp"


struct GameBoard {
    public:
    static constexpr int ColorCount = 2;
    static constexpr int  PieceCount = 6;
    BitBoard all_bitboards[ColorCount][PieceCount] = {};
    Color active_color;

    uint8_t castling_rights = 0b1111;
    Square en_passant_sq = None; // -1 oznacza brak pola do bicia w przelocie
    int halfmove_clock = 0; // Licznik reguły 50-ruchów

    BitBoard& get_bitboard(Color color, PieceType piece) {
        return all_bitboards[static_cast<int>(color)][static_cast<int>(piece)];
    }

    BitBoard get_bitboard(Color color, PieceType piece) const{
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

    struct UndoInfo {
        PieceType moved_piece;
        PieceType captured_piece;
        int castling_rights;
        Square en_passant_square;
        int halfmove_clock;
    };

    static constexpr uint8_t CASTLING_RIGHTS_MASK[64] = {
        13, 15, 15, 15, 12, 15, 15, 14, // Rząd 1: A1 usuwa Q, E1 usuwa K i Q, H1 usuwa K
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        7,  15, 15, 15, 3,  15, 15, 11  // Rząd 8: A8 usuwa q, E8 usuwa k i q, H8 usuwa k
    };

    bool is_square_attacked(Square sq, Color attacker_color) const {
        BitBoard occupied = get_bitboard_all();
        uint64_t sq_bit = 1ull << static_cast<int>(sq);

       Color reverse_color = (attacker_color == Color::White) ? Color::Black : Color::White;

        if (Attacks::get_pawn_attacks(reverse_color, sq).board & get_bitboard(attacker_color, PieceType::Pawn).board) {
            return true;
        }

        // 2. Ataki Skoczków
        // Używamy nowej, szybkiej funkcji z modułu Attacks i pamiętamy o ".board"
        if (Attacks::get_knight_attacks(sq).board & get_bitboard(attacker_color, PieceType::Knight).board) {
            return true;
        }

        // 3. Ataki figur ślizgowych (Wieże, Gońce, Hetmany) korzystające z Magic Bitboards
        uint64_t rooks_queens = get_bitboard(attacker_color, PieceType::Rook).board | get_bitboard(attacker_color, PieceType::Queen).board;
        if (MoveGen::get_rook_attacks(sq, occupied).board & rooks_queens) return true;

        uint64_t bishops_queens = get_bitboard(attacker_color, PieceType::Bishop).board | get_bitboard(attacker_color, PieceType::Queen).board;
        if (MoveGen::get_bishop_attacks(sq, occupied).board & bishops_queens) return true;

        // 4. Ataki Króla
        // Odtworzone dzięki nowej funkcji get_king_attacks
        if (Attacks::get_king_attacks(sq).board & get_bitboard(attacker_color, PieceType::King).board) {
            return true;
        }

        return false;
    }

    bool is_in_check(Color color) const {
        uint64_t king_board = get_bitboard(color, PieceType::King).board;
        if (king_board == 0) return false; // Zabezpieczenie (w poprawnych szachach król zawsze jest)

        // std::countr_zero znajduje indeks zapalonego bitu (pozycję króla 0-63)
        Square king_sq = static_cast<Square>(std::countr_zero(king_board));
        Color enemy_color = (color == Color::White) ? Color::Black : Color::White;

        return is_square_attacked(king_sq, enemy_color);
    }

    void set_bitboard(Color color,PieceType piece, BitBoard board) {
      all_bitboards[static_cast<int>(color)][static_cast<int>(piece)] = board;
    }

   void make_move(const Move& m, UndoInfo& undo) {
        Color enemy_color = (active_color == Color::White) ? Color::Black : Color::White;

        undo.moved_piece = get_piece_at(m.from, active_color);
        undo.captured_piece = get_piece_at(m.to, enemy_color);
        undo.castling_rights = castling_rights;
        undo.en_passant_square = en_passant_sq;
        undo.halfmove_clock = halfmove_clock;

        get_bitboard(active_color, undo.moved_piece).board &= ~(1ull << static_cast<int>(m.from));

        if (undo.captured_piece != PieceType::None && !m.is_en_passant) {
            get_bitboard(enemy_color, undo.captured_piece).board &= ~(1ull << static_cast<int>(m.to));
        }

        PieceType piece_to_place = undo.moved_piece;
        en_passant_sq = None; // Resetujemy pole bicia w przelocie po każdym ruchu
        halfmove_clock++;   // Zakładamy, że to był spokojny ruch

        // -- OBSŁUGA RUCHÓW SPECJALNYCH --

        if (m.promotion != PieceType::None) {
            piece_to_place = m.promotion; // Kładziemy na polu np. Hetmana
        }

        if (m.is_en_passant) {
            undo.captured_piece = PieceType::Pawn; // Ręcznie oznaczamy bicie piona dla cofania
            // Musimy usunąć piona z rzędu wyżej/niżej, a nie z pola m.to
            int ep_pawn_sq = (active_color == Color::White) ? m.to - 8 : m.to + 8;
            get_bitboard(enemy_color, PieceType::Pawn).board &= ~(1ull << ep_pawn_sq);
        }

        if (m.is_castling) {
            int rook_from, rook_to;
            if (m.to == 6)       { rook_from = 7;  rook_to = 5; }  // Biała krótka (g1)
            else if (m.to == 2)  { rook_from = 0;  rook_to = 3; }  // Biała długa (c1)
            else if (m.to == 62) { rook_from = 63; rook_to = 61; } // Czarna krótka (g8)
            else if (m.to == 58) { rook_from = 56; rook_to = 59; } // Czarna długa (c8)

            // Przestawiamy wieżę
            get_bitboard(active_color, PieceType::Rook).board ^= (1ull << rook_from) | (1ull << rook_to);
        }

        if (undo.moved_piece == PieceType::Pawn) {
            halfmove_clock = 0;
            const int diff = m.to - m.from;

            if (diff == 16)  en_passant_sq = static_cast<Square>(m.from + 8);
            if (diff == -16) en_passant_sq = static_cast<Square>(m.from - 8);
        }

        if (undo.captured_piece != PieceType::None) {
            halfmove_clock = 0; // Bicie również resetuje regułę 50-ruchów
        }

        // 4. Postaw figurę na polu docelowym
        get_bitboard(active_color, piece_to_place).board |= (1ull << static_cast<int>(m.to));

        // 5. Aktualizacja praw do roszady i tury
        castling_rights &= CASTLING_RIGHTS_MASK[m.from];
        castling_rights &= CASTLING_RIGHTS_MASK[m.to];
        active_color = enemy_color;
    }


    void unmake_move(const Move& m, const UndoInfo& undo) {
        // Wracamy do naszego koloru (cofamy zmianę tury)
        active_color = (active_color == Color::White) ? Color::Black : Color::White;
        Color enemy_color = (active_color == Color::White) ? Color::Black : Color::White;

        // Jeśli to była promocja, to zdejmujemy z planszy promowaną figurę, w innym przypadku piona/inną figurę
        PieceType piece_on_to_sq = (m.promotion != PieceType::None) ? m.promotion : undo.moved_piece;

        // 1. Zdejmij naszą figurę z pola docelowego
        get_bitboard(active_color, piece_on_to_sq).board &= ~(1ull << static_cast<int>(m.to));

        // 2. Postaw naszą pierwotną figurę z powrotem na pole startowe
        get_bitboard(active_color, undo.moved_piece).board |= (1ull << static_cast<int>(m.from));

        // 3. Przywróć zbite figury
        if (m.is_en_passant) {
            // Wrogiego piona trzeba postawić za polem bicia, nie na m.to
            int ep_pawn_sq = (active_color == Color::White) ? m.to - 8 : m.to + 8;
            get_bitboard(enemy_color, PieceType::Pawn).board |= (1ull << ep_pawn_sq);
        }
        else if (undo.captured_piece != PieceType::None) {
            // Zwykłe bicie - wroga figura wraca na pole docelowe m.to
            get_bitboard(enemy_color, undo.captured_piece).board |= (1ull << static_cast<int>(m.to));
        }

        // 4. Cofnij ruch wieży przy roszadzie
        if (m.is_castling) {
            int rook_from, rook_to;
            if (m.to == 6)       { rook_from = 7;  rook_to = 5; }
            else if (m.to == 2)  { rook_from = 0;  rook_to = 3; }
            else if (m.to == 62) { rook_from = 63; rook_to = 61; }
            else if (m.to == 58) { rook_from = 56; rook_to = 59; }

            // Przesuwamy wieżę na swoje miejsce startowe za pomocą XORa
            get_bitboard(active_color, PieceType::Rook).board ^= (1ull << rook_from) | (1ull << rook_to);
        }

        // 5. Odtwórz pełen stan gry ze struktury undo
        castling_rights = undo.castling_rights;
        en_passant_sq = undo.en_passant_square;
        halfmove_clock = undo.halfmove_clock;
    }

    PieceType get_piece_at(Square sq, Color color) {
        uint64_t bit = 1ull << static_cast<int>(sq);
        if (get_bitboard(color, PieceType::Pawn).board & bit)   return PieceType::Pawn;
        if (get_bitboard(color, PieceType::Knight).board & bit) return PieceType::Knight;
        if (get_bitboard(color, PieceType::Bishop).board & bit) return PieceType::Bishop;
        if (get_bitboard(color, PieceType::Rook).board & bit)   return PieceType::Rook;
        if (get_bitboard(color, PieceType::Queen).board & bit)  return PieceType::Queen;
        if (get_bitboard(color, PieceType::King).board & bit)   return PieceType::King;
        return PieceType::None;
    }

    int evaluate_board(const GameBoard& board) {
    // Standardowe wartdości figur w centypionach (1 pion = 100)
    constexpr int PAWN_VAL   = 100;
    constexpr int KNIGHT_VAL = 300;
    constexpr int BISHOP_VAL = 300;
    constexpr int ROOK_VAL   = 500;
    constexpr int QUEEN_VAL  = 900;

    int white_material = 0;
    int black_material = 0;
    white_material += std::popcount(board.get_bitboard(Color::White, PieceType::Pawn).board)   * PAWN_VAL;
    white_material += std::popcount(board.get_bitboard(Color::White, PieceType::Knight).board) * KNIGHT_VAL;
    white_material += std::popcount(board.get_bitboard(Color::White, PieceType::Bishop).board) * BISHOP_VAL;
    white_material += std::popcount(board.get_bitboard(Color::White, PieceType::Rook).board)   * ROOK_VAL;
    white_material += std::popcount(board.get_bitboard(Color::White, PieceType::Queen).board)  * QUEEN_VAL;

    black_material += std::popcount(board.get_bitboard(Color::Black, PieceType::Pawn).board)   * PAWN_VAL;
    black_material += std::popcount(board.get_bitboard(Color::Black, PieceType::Knight).board) * KNIGHT_VAL;
    black_material += std::popcount(board.get_bitboard(Color::Black, PieceType::Bishop).board) * BISHOP_VAL;
    black_material += std::popcount(board.get_bitboard(Color::Black, PieceType::Rook).board)   * ROOK_VAL;
    black_material += std::popcount(board.get_bitboard(Color::Black, PieceType::Queen).board)  * QUEEN_VAL;

    const int evaluation = white_material - black_material;

    return (board.active_color == Color::White) ? evaluation : -evaluation;
}

    static GameBoard starting_position() {
        GameBoard board;
        board.active_color = Color::White;
        board.set_bitboard(Color::White, PieceType::Pawn,   BitBoard(0xFF00ull));
        board.set_bitboard(Color::White, PieceType::Rook,   BitBoard(0x0081ull));
        board.set_bitboard(Color::White, PieceType::Knight, BitBoard(0x0042ull));
        board.set_bitboard(Color::White, PieceType::Bishop, BitBoard(0x0024ull));
        board.set_bitboard(Color::White, PieceType::Queen,  BitBoard(0x0008ull));
        board.set_bitboard(Color::White, PieceType::King,   BitBoard(0x0010ull));
        board.set_bitboard(Color::Black, PieceType::Pawn,   BitBoard(0xFF000000000000ull));
        board.set_bitboard(Color::Black, PieceType::Rook,   BitBoard(0x8100000000000000ull));
        board.set_bitboard(Color::Black, PieceType::Knight, BitBoard(0x4200000000000000ull));
        board.set_bitboard(Color::Black, PieceType::Bishop, BitBoard(0x2400000000000000ull));
        board.set_bitboard(Color::Black, PieceType::Queen,  BitBoard(0x0800000000000000ull));
        board.set_bitboard(Color::Black, PieceType::King,   BitBoard(0x1000000000000000ull));
        return board;
    }




};


#endif //BITBOARD_CHESS_GAMEBOARD_HPP
