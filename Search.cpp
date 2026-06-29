
#include "Search.hpp"

#include "MoveGen.hpp"
#include "move.hpp"

namespace Search {



    int negamax(GameBoard& board, int depth, int alpha, int beta) {
        if (depth == 0) {
            return quiescence_search(board, alpha, beta);
        }

        MoveList move_list = MoveGen::get_all_moves(board, board.active_color);

        int max_score = -INF;
        int legal_moves = 0; // Zliczamy tylko legalne ruchy

        for (int i = 0; i < move_list.count; ++i) {
            Move move = move_list.moves[i];
            GameBoard::UndoInfo undo;

            board.make_move(move, undo);

            // --- FILTROWANIE NIELEGALNYCH RUCHÓW ---
            // Po wykonaniu ruchu tura zmienia się na przeciwnika w make_move[cite: 1].
            // Musimy sprawdzić, czy kolor, który WYKONAŁ ruch, jest w szachu.
            Color my_color = (board.active_color == Color::White) ? Color::Black : Color::White;
            if (board.is_in_check(my_color)) {
                board.unmake_move(move, undo);
                continue; // Król jest pod biciem, ignorujemy ten ruch
            }

            legal_moves++; // Znaleźliśmy w pełni legalny ruch

            int score = -negamax(board, depth - 1, -beta, -alpha);

            board.unmake_move(move, undo);

            if (score > max_score) {
                max_score = score;
            }

            if (max_score > alpha) {
                alpha = max_score;
            }

            if (alpha >= beta) {
                break; // Odcięcie Beta
            }
        }

        // --- SPRAWDZENIE MATA I PATA ---
        if (legal_moves == 0) {
            // Skoro nie ma legalnych ruchów, sprawdzamy, czy obecny gracz jest w szachu
            if (board.is_in_check(board.active_color)) {
                // MAT. Zwracamy głębokość, aby silnik preferował znalezienie mata szybciej (np. w 1 ruchu niż w 5)
                return -MATE_SCORE + depth;
            } else {
                // PAT (brak ruchów, ale królowi nic nie grozi) - ocena remisowa
                return 0;
            }
        }

        return max_score;
    }

    Move find_best_move(GameBoard& board, int depth) {
        MoveList move_list = MoveGen::get_all_moves(board, board.active_color);

        // Inicjalizujemy best_move jako pusty, na wypadek gdyby nie było legalnych ruchów
        Move best_move = Move{None, None};
        int best_score = -INF;
        int alpha = -INF;
        int beta = INF;

        for (int i = 0; i < move_list.count; ++i) {
            Move move = move_list.moves[i];
            GameBoard::UndoInfo undo;

            board.make_move(move, undo);

            // Filtrowanie ruchów również musi mieć miejsce na głównym poziomie wyszukiwania
            Color my_color = (board.active_color == Color::White) ? Color::Black : Color::White;
            if (board.is_in_check(my_color)) {
                board.unmake_move(move, undo);
                continue;
            }

            int score = -negamax(board, depth - 1, -beta, -alpha);
            board.unmake_move(move, undo);

            // Jeśli to nasz pierwszy poprawny ruch lub ma lepszą ocenę, nadpisujemy best_move
            if (score > best_score || best_move.from == None) {
                best_score = score;
                best_move = move;
            }
            if (score > alpha) {
                alpha = score;
            }
        }

        return best_move;
    }


    inline int quiescence_search(GameBoard& board, int alpha, int beta) {
        int stand_pat = board.evaluate_board(board);

        if (stand_pat >= beta) {
            return beta;
        }
        if (alpha < stand_pat) {
            alpha = stand_pat;
        }

        MoveList move_list = MoveGen::get_all_moves(board, board.active_color);
        Color enemy_color = (board.active_color == Color::White) ? Color::Black : Color::White;

        for (int i = 0; i < move_list.count; ++i) {
            Move move = move_list.moves[i];

            if (board.get_piece_at(move.to, enemy_color) == PieceType::None && !move.is_en_passant) {
                continue;
            }

            GameBoard::UndoInfo undo;
            board.make_move(move, undo);

            Color my_color = (board.active_color == Color::White) ? Color::Black : Color::White;
            if (board.is_in_check(my_color)) {
                board.unmake_move(move, undo);
                continue;
            }

            int score = -quiescence_search(board, -beta, -alpha);

            board.unmake_move(move, undo);

            if (score >= beta) {
                return beta;
            }
            if (score > alpha) {
                alpha = score;
            }
        }
        return alpha;
    }
}
