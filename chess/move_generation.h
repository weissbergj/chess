#ifndef MOVE_GENERATION_H
#define MOVE_GENERATION_H

void generate_knight_moves(char board[8][8], int row, int col, int moves[][4], int *move_count);
void generate_pawn_moves(char board[8][8], int row, int col, int moves[][4], int *move_count);
void generate_sliding_moves(char board[8][8], int row, int col, const int directions[][2], 
                            int num_directions, int moves[][4], int *move_count);
void generate_rook_moves(char board[8][8], int row, int col, int moves[][4], int *move_count);
void generate_bishop_moves(char board[8][8], int row, int col, int moves[][4], int *move_count);
void generate_queen_moves(char board[8][8], int row, int col, int moves[][4], int *move_count);
void parse_move(char *input, int *src_row, int *src_col, int *dest_row, int *dest_col);
void generate_king_moves(char board[8][8], int row, int col, int moves[][4], int *move_count);

void make_move(char board[8][8], int src_row, int src_col, int dest_row, int dest_col, int is_real_move);

#endif // MOVE_GENERATION_H