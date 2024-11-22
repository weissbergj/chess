#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

int is_game_over(char board[8][8], int *is_checkmate, int is_white_turn);
int is_in_check(char board[8][8], int is_white_turn);
int has_legal_moves(char board[8][8], int is_white_turn);
int move_causes_check(char board[8][8], int src_row, int src_col, int dest_row, int dest_col);
int can_castle_kingside(char board[8][8], int is_white_turn);
int can_castle_queenside(char board[8][8], int is_white_turn);
int is_square_attacked(char board[8][8], int row, int col, int is_white_turn);

#endif // GAME_LOGIC_H