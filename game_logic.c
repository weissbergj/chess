#include "game_logic.h"
#include "move_validation.h"
#include "game_state.h"
#include "board.h"
#include "utils.h"
#include "strings.h"

// Check if the game is over due to checkmate, stalemate, or other conditions
int is_game_over(char board[8][8], int *is_checkmate, int white_player) {
    int white_king_found = 0;
    int black_king_found = 0;

    // Check for king presence
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            char piece = board[i][j];
            if (piece == 'K') white_king_found = 1;
            if (piece == 'k') black_king_found = 1;
        }
    }

    if (!white_king_found || !black_king_found) {
        *is_checkmate = 1;
        return 1;
    }

    // Check for insufficient material
    if (is_insufficient_material(board)) {
        *is_checkmate = 0;
        return 1;
    }

    // Check for 50-move rule
    if (moves_without_capture >= 100) {
        *is_checkmate = 0;
        return 1;
    }

    // Check for checkmate or stalemate
    if (!has_legal_moves(board, white_player)) {
        *is_checkmate = is_in_check(board, white_player);
        return 1;
    }

    return 0;
}

// Determine if there are any legal moves for the current player
int has_legal_moves(char board[8][8], int white_player) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            char piece = board[i][j];
            if ((white_player && is_white_piece(piece)) || (!white_player && is_black_piece(piece))) {
                for (int di = 0; di < 8; di++) {
                    for (int dj = 0; dj < 8; dj++) {
                        if (is_valid_move(board, i, j, di, dj)) {
                            return 1;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

// // Determine if there are any legal moves for the current player
// int has_legal_moves(char board[8][8], int white_player) {
//     for (int i = 0; i < 8; i++) {
//         for (int j = 0; j < 8; j++) {
//             char piece = board[i][j];
//             if ((white_player && is_white_piece(piece)) || (!white_player && is_black_piece(piece))) {
//                 // Check all possible moves for the current piece
//                 for (int di = 0; di < 8; di++) {
//                     for (int dj = 0; dj < 8; dj++) {
//                         // Check if the move is valid
//                         if (is_valid_move(board, i, j, di, dj)) {
//                             // Temporarily make the move
//                             char temp = board[di][dj];
//                             board[di][dj] = piece;  // Move the piece
//                             board[i][j] = '.';       // Empty the original square

//                             // Check if the move leaves the king in check
//                             int is_check = is_in_check(board, white_player);
                            
//                             // Undo the move
//                             board[i][j] = piece;  // Restore the original piece
//                             board[di][dj] = temp;  // Restore the original square

//                             // If the move is valid and does not leave the king in check, return true
//                             if (!is_check) {
//                                 return 1;  // Legal move found
//                             }
//                         }
//                     }
//                 }
//             }
//         }
//     }
//     return 0;  // No legal moves found
// }

// Check if a square is attacked by the opponent
int is_square_attacked(char board[8][8], int row, int col, int by_white) {
    // Check for pawn attacks
    int pawn_direction = by_white ? 1 : -1;
    if (is_within_bounds(row + pawn_direction, col - 1) && 
        ((by_white && board[row + pawn_direction][col - 1] == 'P') || 
         (!by_white && board[row + pawn_direction][col - 1] == 'p'))) {
        return 1;
    }
    if (is_within_bounds(row + pawn_direction, col + 1) && 
        ((by_white && board[row + pawn_direction][col + 1] == 'P') || 
         (!by_white && board[row + pawn_direction][col + 1] == 'p'))) {
        return 1;
    }

    // Check for knight attacks
    static const int knight_moves[8][2] = {
        {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
        {1, -2}, {1, 2}, {2, -1}, {2, 1}
    };
    for (int i = 0; i < 8; i++) {
        int new_row = row + knight_moves[i][0];
        int new_col = col + knight_moves[i][1];
        if (is_within_bounds(new_row, new_col) && 
            ((by_white && board[new_row][new_col] == 'N') || 
             (!by_white && board[new_row][new_col] == 'n'))) {
            return 1;
        }
    }

    // Check for sliding piece attacks (queen, rook, bishop)
    static const int directions[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        {0, -1},           {0, 1},
        {1, -1},  {1, 0},  {1, 1}
    };
    for (int d = 0; d < 8; d++) {
        int new_row = row + directions[d][0];
        int new_col = col + directions[d][1];
        while (is_within_bounds(new_row, new_col)) {
            char piece = board[new_row][new_col];
            if (piece != '.') {
                if (by_white) {
                    if (piece == 'Q' || 
                        (piece == 'R' && (directions[d][0] == 0 || directions[d][1] == 0)) ||
                        (piece == 'B' && (directions[d][0] != 0 && directions[d][1] != 0))) {
                        return 1;
                    }
                } else {
                    if (piece == 'q' || 
                        (piece == 'r' && (directions[d][0] == 0 || directions[d][1] == 0)) ||
                        (piece == 'b' && (directions[d][0] != 0 && directions[d][1] != 0))) {
                        return 1;
                    }
                }
                break;
            }
            new_row += directions[d][0];
            new_col += directions[d][1];
        }
    }

    // Check for king attacks
    for (int d = 0; d < 8; d++) {
        int new_row = row + directions[d][0];
        int new_col = col + directions[d][1];
        if (is_within_bounds(new_row, new_col) && 
            ((by_white && board[new_row][new_col] == 'K') || 
             (!by_white && board[new_row][new_col] == 'k'))) {
            return 1;
        }
    }

    return 0;
}

// Check if the player's king is in check
int is_in_check(char board[8][8], int white_player) {
    int king_row, king_col;
    find_king(board, &king_row, &king_col, white_player);
    return is_square_attacked(board, king_row, king_col, !white_player);
}

// Determine if a move would leave the player in check
int move_causes_check(char board[8][8], int src_row, int src_col, int dest_row, int dest_col) {
    char temp_board[8][8];
    memcpy(temp_board, board, sizeof(temp_board));

    char piece = temp_board[src_row][src_col];
    temp_board[dest_row][dest_col] = piece;
    temp_board[src_row][src_col] = '.';

    return is_in_check(temp_board, is_white_piece(piece));
}

// Validate kingside castling
int can_castle_kingside(char board[8][8], int is_white) {
    int row = is_white ? 7 : 0;
    char king = is_white ? 'K' : 'k';
    char rook = is_white ? 'R' : 'r';

    if (board[row][4] != king || board[row][7] != rook) return 0;
    if (is_white ? white_king_moved : black_king_moved) return 0;
    if (is_white ? white_rooks_moved[1] : black_rooks_moved[1]) return 0;
    if (board[row][5] != '.' || board[row][6] != '.') return 0;

    for (int col = 4; col <= 6; col++) {
        if (is_square_attacked(board, row, col, !is_white)) return 0;
    }

    return 1;
}

// Validate queenside castling
int can_castle_queenside(char board[8][8], int is_white) {
    int row = is_white ? 7 : 0;
    char king = is_white ? 'K' : 'k';
    char rook = is_white ? 'R' : 'r';

    if (board[row][4] != king || board[row][0] != rook) return 0;
    if (is_white ? white_king_moved : black_king_moved) return 0;
    if (is_white ? white_rooks_moved[0] : black_rooks_moved[0]) return 0;
    if (board[row][1] != '.' || board[row][2] != '.' || board[row][3] != '.') return 0;

    for (int col = 2; col <= 4; col++) {
        if (is_square_attacked(board, row, col, !is_white)) return 0;
    }

    return 1;
}