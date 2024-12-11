#ifndef MOVE_SORT_H
#define MOVE_SORT_H

int evaluate_castling(char piece, int src_col, int dest_col);
int evaluate_captures(char captured);
int move_sort(char board[8][8], int src_row, int src_col, int dest_row, int dest_col, int move_count);

#endif // MOVE_SORT_H