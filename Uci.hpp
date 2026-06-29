#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include "Gameboard.hpp"
#include "MoveGen.hpp"
#include "Search.hpp"

namespace UCI {

    // Zamienia strukturę Move na string w formacie UCI (np. "e2e4" lub "e7e8q")
    inline std::string move_to_string(const Move& m) {
        if (m.from == None) return "0000";
        
        std::string s;
        s += static_cast<char>('a' + (m.from % 8)); // Kolumna startowa
        s += static_cast<char>('1' + (m.from / 8)); // Rząd startowy
        s += static_cast<char>('a' + (m.to % 8));   // Kolumna docelowa
        s += static_cast<char>('1' + (m.to / 8));   // Rząd docelowy

        // Obsługa promocji
        if (m.promotion == PieceType::Queen) s += 'q';
        else if (m.promotion == PieceType::Rook) s += 'r';
        else if (m.promotion == PieceType::Bishop) s += 'b';
        else if (m.promotion == PieceType::Knight) s += 'n';

        return s;
    }

    // Szuka ruchu pasującego do stringa (np. "e2e4") na liście legalnych ruchów
    inline Move parse_move(const std::string& move_str, GameBoard& board) {
        MoveList list = MoveGen::get_all_moves(board, board.active_color);
        for (int i = 0; i < list.count; i++) {
            Move m = list.moves[i];
            

            GameBoard::UndoInfo undo{};
            board.make_move(m, undo);
            Color my_color = (board.active_color == Color::White) ? Color::Black : Color::White;
            bool is_legal = !board.is_in_check(my_color);
            board.unmake_move(m, undo);

            if (is_legal && move_to_string(m) == move_str) {
                return m;
            }
        }
        return Move{None, None};
    }

    // Główna pętla nasłuchująca komend z GUI
    inline void loop() {
        std::string line;
        GameBoard board = GameBoard::starting_position();

        std::cout << "id name PiotrekChess 1.0\n";
        std::cout << "id author Piotrek\n";
        std::cout << "uciok\n";

        while (std::getline(std::cin, line)) {
            std::istringstream iss(line);
            std::string token;
            iss >> token;

            if (token == "quit") {
                break;
            } 
            else if (token == "uci") {
                std::cout << "id name PiotrekChess 1.0\n";
                std::cout << "id author Piotrek\n";
                std::cout << "uciok\n";
            } 
            else if (token == "isready") {
                std::cout << "readyok\n";
            } 
            else if (token == "ucinewgame") {
                board = GameBoard::starting_position();
            } 
            else if (token == "position") {
                iss >> token; // pobiera "startpos" lub "fen"
                
                if (token == "startpos") {
                    board = GameBoard::starting_position();
                }
                
                iss >> token; // Sprawdza czy jest słowo "moves"
                if (token == "moves") {
                    std::string move_str;
                    while (iss >> move_str) {
                        Move m = parse_move(move_str, board);
                        if (m.from != None) {
                            GameBoard::UndoInfo undo{};
                            board.make_move(m, undo);
                        }
                    }
                }
            } 
            else if (token == "go") {
                int depth = 5;
                std::string param;

                while (iss >> param) {
                    if (param == "depth") {
                        iss >> depth;
                    }
                    else if (param == "wtime" || param == "btime" || param == "movetime") {
                        // TODO: zarządzanie czasem.
                    }
                }

                // Dajemy znać GUI o parametrach (opcjonalne, ale fajnie wygląda w konsoli)
                std::cout << "info depth " << depth << "\n";

                // Wywołujemy szukanie z dynamiczną głębokością
                Move best_move = Search::find_best_move(board, depth);

                // Zwracamy odpowiedź do GUI
                std::cout << "bestmove " << move_to_string(best_move) << "\n";
            }
        }
    }
}