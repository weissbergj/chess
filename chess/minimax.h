#ifndef MINIMAX_H
#define MINIMAX_H

#include "game_state.h"

int minimax(char board[8][8], int depth, int alpha, int beta, int maximizing_player, int current_move_count);
void find_best_move(char board[8][8], int best_move[4], int is_white);

#endif // MINIMAX_H