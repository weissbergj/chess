#ifndef MOVE_VALIDATION_H
#define MOVE_VALIDATION_H

int is_valid_bishop_move(char board[8][8], int from_row, int from_col, int to_row, int to_col);
int is_valid_king_move(char board[8][8], int from_row, int from_col, int to_row, int to_col);
int is_valid_knight_move(char board[8][8], int from_row, int from_col, int to_row, int to_col);
int is_valid_pawn_move(char board[8][8], int from_row, int from_col, int to_row, int to_col);
int is_valid_queen_move(char board[8][8], int from_row, int from_col, int to_row, int to_col);
int is_valid_rook_move(char board[8][8], int from_row, int from_col, int to_row, int to_col);
int is_valid_move(char board[8][8], int from_row, int from_col, int to_row, int to_col);
int is_valid_en_passant(char board[8][8], int from_row, int from_col, int to_row, int to_col);
int is_valid_input_format(char *input);

#endif // MOVE_VALIDATION_H