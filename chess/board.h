#ifndef BOARD_H
#define BOARD_H

#include "game_state.h"

void display_board(char board[8][8]);
void display_move(int src_row, int src_col, int dest_row, int dest_col);
int find_king(char board[8][8], int *king_row, int *king_col, int is_white);
int count_pieces(char board[8][8]);
int is_within_bounds(int row, int col);

#endif // BOARD_H