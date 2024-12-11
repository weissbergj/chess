#ifndef MOVE_SCORING_H
#define MOVE_SCORING_H

#include "utils.h"  // For tolower function
#include "strings.h" // For memcpy function

int evaluate_position(char piece, int row, int col, int current_move_count, char board[8][8]);
int evaluate_board(char board[8][8], int current_move_count);

// Helper functions for piece evaluations
int evaluate_pawn(int row, int col, int current_move_count);
int evaluate_knight(int row, int col);
int evaluate_bishop(int row, int col);
int evaluate_rook(int row, int col);
int evaluate_queen(int row, int col, int current_move_count);
int evaluate_king(int row, int col, int current_move_count);

// OTHER
int evaluate_king_safety(char board[8][8], int is_white);
int evaluate_pawn_shield(char board[8][8], int king_row, int king_col, int is_white, int early_game);

#endif // MOVE_SCORING_H