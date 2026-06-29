//
// Created by Piotrek on 28.06.2026.
//

#ifndef BITBOARD_CHESS_SEARCH_HPP
#define BITBOARD_CHESS_SEARCH_HPP

#pragma once
#include <limits>
#include "Gameboard.hpp"

#include "move.hpp"

namespace Search {

    constexpr int INF = std::numeric_limits<int>::max();
    constexpr int MATE_SCORE = 99999;

    int negamax(GameBoard& board, int depth, int alpha, int beta);
    inline int quiescence_search(GameBoard& board, int alpha, int beta);

    Move find_best_move(GameBoard& board, int depth);
    }
#endif //BITBOARD_CHESS_SEARCH_HPP