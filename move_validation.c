#include "move_validation.h"
#include "game_state.h"
#include "game_logic.h"
#include "move_generation.h"
#include "board.h"
#include "utils.h"
#include "strings.h"

// Validate pawn moves, including normal, initial two-square, and capture moves
int is_valid_pawn_move(char board[8][8], int src_row, int src_col, int dest_row, int dest_col) {
    char piece = board[src_row][src_col];
    int direction = is_white_piece(piece) ? -1 : 1;
    int start_row = is_white_piece(piece) ? 6 : 1;

    if (src_col == dest_col) {
        if (dest_row == src_row + direction) {
            return board[dest_row][dest_col] == '.';
        }
        if (src_row == start_row && dest_row == src_row + 2 * direction) {
            return board[dest_row][dest_col] == '.' && board[src_row + direction][src_col] == '.';
        }
    } else if (abs(src_col - dest_col) == 1 && dest_row == src_row + direction) {
        return board[dest_row][dest_col] != '.' && can_capture(piece, board[dest_row][dest_col]);
    }

    return 0;
}

// Validate rook moves, ensuring path is clear
int is_valid_rook_move(char board[8][8], int src_row, int src_col, int dest_row, int dest_col) {
    if (src_row != dest_row && src_col != dest_col) return 0;

    int row_step = (dest_row > src_row) ? 1 : (dest_row < src_row) ? -1 : 0;
    int col_step = (dest_col > src_col) ? 1 : (dest_col < src_col) ? -1 : 0;

    for (int r = src_row + row_step, c = src_col + col_step; r != dest_row || c != dest_col; r += row_step, c += col_step) {
        if (board[r][c] != '.') return 0;
    }

    return can_capture(board[src_row][src_col], board[dest_row][dest_col]);
}

// Validate knight moves based on L-shape pattern
int is_valid_knight_move(char board[8][8], int src_row, int src_col, int dest_row, int dest_col) {
    int row_diff = abs(dest_row - src_row);
    int col_diff = abs(dest_col - src_col);
    return (row_diff == 2 && col_diff == 1) || (row_diff == 1 && col_diff == 2);
}

// Validate bishop moves, ensuring path is clear
int is_valid_bishop_move(char board[8][8], int src_row, int src_col, int dest_row, int dest_col) {
    if (abs(dest_row - src_row) != abs(dest_col - src_col)) return 0;

    int row_step = (dest_row > src_row) ? 1 : -1;
    int col_step = (dest_col > src_col) ? 1 : -1;

    for (int r = src_row + row_step, c = src_col + col_step; r != dest_row; r += row_step, c += col_step) {
        if (board[r][c] != '.') return 0;
    }

    return can_capture(board[src_row][src_col], board[dest_row][dest_col]);
}

// Validate queen moves, combining rook and bishop logic
int is_valid_queen_move(char board[8][8], int src_row, int src_col, int dest_row, int dest_col) {
    return is_valid_rook_move(board, src_row, src_col, dest_row, dest_col) ||
           is_valid_bishop_move(board, src_row, src_col, dest_row, dest_col);
}

// Validate king moves, including castling
int is_valid_king_move(char board[8][8], int src_row, int src_col, int dest_row, int dest_col) {
    return abs(dest_row - src_row) <= 1 && abs(dest_col - src_col) <= 1;
}

// General move validation, including special rules like en passant and castling
int is_valid_move(char board[8][8], int src_row, int src_col, int dest_row, int dest_col) {
    if (!is_within_bounds(src_row, src_col) || !is_within_bounds(dest_row, dest_col)) return 0;

    char piece = board[src_row][src_col];
    if (piece == '.') return 0;

    if (board[dest_row][dest_col] != '.' && is_white_piece(piece) == is_white_piece(board[dest_row][dest_col])) return 0;

    int valid = 0;
    switch (tolower(piece)) {
        case 'p':
            valid = is_valid_pawn_move(board, src_row, src_col, dest_row, dest_col);
            if (!valid && src_col != dest_col && board[dest_row][dest_col] == '.') {
                valid = is_valid_en_passant(board, src_row, src_col, dest_row, dest_col);
            }
            break;
        case 'r': valid = is_valid_rook_move(board, src_row, src_col, dest_row, dest_col); break;
        case 'n': valid = is_valid_knight_move(board, src_row, src_col, dest_row, dest_col); break;
        case 'b': valid = is_valid_bishop_move(board, src_row, src_col, dest_row, dest_col); break;
        case 'q': valid = is_valid_queen_move(board, src_row, src_col, dest_row, dest_col); break;
        case 'k':
            if (abs(dest_col - src_col) == 2) {
                valid = (dest_col > src_col) ? can_castle_kingside(board, is_white_piece(piece)) : can_castle_queenside(board, is_white_piece(piece));
            } else {
                valid = is_valid_king_move(board, src_row, src_col, dest_row, dest_col);
            }
            break;
        default: return 0;
    }

    if (!valid) return 0;

    char temp_board[8][8];
    memcpy(temp_board, board, sizeof(temp_board));
    temp_board[dest_row][dest_col] = piece;
    temp_board[src_row][src_col] = '.';

    if (tolower(piece) == 'p' && src_col != dest_col && board[dest_row][dest_col] == '.') {
        temp_board[src_row][dest_col] = '.';
    }

    return !is_in_check(temp_board, is_white_piece(piece));
}

// Validate en passant move
int is_valid_en_passant(char board[8][8], int src_row, int src_col, int dest_row, int dest_col) {
    if (move_count == 0) return 0;

    Move last_move = move_history[move_count - 1];
    char piece = board[src_row][src_col];

    if (tolower(last_move.piece) != 'p' || abs(last_move.from_row - last_move.to_row) != 2) return 0;

    if (is_white_piece(piece)) {
        if (src_row != 3 || dest_row != 2) return 0;
    } else {
        if (src_row != 4 || dest_row != 5) return 0;
    }

    return (dest_col == last_move.to_col && abs(src_col - dest_col) == 1);
}

// Validate input format for move strings
int is_valid_input_format(char *input) {
    return strlen(input) == 4 &&
           input[0] >= 'a' && input[0] <= 'h' &&
           input[1] >= '1' && input[1] <= '8' &&
           input[2] >= 'a' && input[2] <= 'h' &&
           input[3] >= '1' && input[3] <= '8';
}