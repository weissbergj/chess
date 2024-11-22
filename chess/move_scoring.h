#ifndef MOVE_SCORING_H
#define MOVE_SCORING_H

#include "utils.h"  // For tolower function
#include "strings.h" // For memcpy function

// Function prototypes
int score_move(char board[8][8], int src_row, int src_col, int dest_row, int dest_col, int move_count);
int evaluate_position(char piece, int row, int col);
int evaluate_board(char board[8][8], int current_move_count);
int evaluate_king_safety(char board[8][8], int is_white);

// Helper function prototypes
int evaluate_knight_development(char piece, int src_row, int dest_row, int dest_col);
int evaluate_pawn_moves(char piece, int src_row, int src_col, int dest_row);
int evaluate_castling(char piece, int src_col, int dest_col);
int evaluate_captures(char captured);

#endif // MOVE_SCORING_H