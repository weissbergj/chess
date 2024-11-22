#include "minimax.h"
#include "move_generation.h"
#include "move_validation.h"
#include "board.h"
#include "move_scoring.h"
#include "game_logic.h"
#include "game_state.h"
#include "move_sort.h"
#include "utils.h"
#include "timer.h"

int evaluate_piece_captures(char board[8][8], int row, int col, char piece, int is_white);

// int is_repetitive_move(const int move[4], const int move_history[][4], int history_count) {
//     if (history_count < 2) return 0; // Not enough history
//     const int *last_move = move_history[history_count - 1];
//     const int *second_last_move = move_history[history_count - 2];
//     // Check if current move reverses the previous move
//     return (move[0] == second_last_move[2] && move[1] == second_last_move[3] &&
//             move[2] == second_last_move[0] && move[3] == second_last_move[1]);
// }

// void penalize_repeated_moves(int *score, const int move[4], const int move_history[][4], int history_count) {
//     if (is_repetitive_move(move, move_history, history_count)) {
//         *score -= 500; // Arbitrary penalty for repeated moves
//     }
// }

// Evaluate pawn shield
int evaluate_pawn_shield(char board[8][8], int king_row, int king_col, int is_white, int early_game) {
    int score = 0;
    int pawn_rank = is_white ? king_row - 1 : king_row + 1;
    if (pawn_rank >= 0 && pawn_rank < 8) {
        for (int col = max(0, king_col - 1); col <= min(7, king_col + 1); col++) {
            if (board[pawn_rank][col] == (is_white ? 'P' : 'p')) {
                score += early_game ? 40 : 20; // Adjusted bonus for pawn shield
            }
        }
    }
    return score;
}

// Evaluate king safety
int evaluate_king_safety(char board[8][8], int is_white) {
    int king_row, king_col;
    if (!find_king(board, &king_row, &king_col, is_white)) return 0; // Safety check

    int safety_score = 0;
    int early_game = move_count < 20;
    int start_rank = is_white ? 7 : 0;

    // Penalize early king movement
    if (early_game && king_row != start_rank) safety_score -= 10 * abs(king_row - start_rank);

    // Pawn shield
    safety_score += evaluate_pawn_shield(board, king_row, king_col, is_white, early_game);

    // Penalize open files in late game
    if (!early_game) {
        for (int col = max(0, king_col - 1); col <= min(7, king_col + 1); col++) {
            if (is_open_file(board, col, is_white)) safety_score -= 20;
        }
    }

    return is_white ? safety_score : -safety_score;
}

int evaluate_pawn(int row, int col, int current_move_count) {
    int score = 0;
    char piece = board[row][col];

    // Enhance pawn structure evaluation
    if (col >= 2 && col <= 5) {
        score += 2;  // Center control
    }
    if ((col == 3 || col == 4) && (row >= 2 && row <= 5)) {
        score += 3;  // Stronger center pawn
    }

    // Stronger advancement incentive in endgame
    int endgame = (current_move_count > 30);
    score += is_white_piece(piece) ? (endgame ? (7 - row) : (7 - row) / 2) : (endgame ? row : row / 2);

    // Penalize doubled pawns
    int doubled = 0;
    for (int r = 0; r < 8; r++) {
        if (r != row && tolower(board[r][col]) == 'p') {
            doubled = 1;
            break;
        }
    }
    if (doubled) score -= 5;
    return score;
}

int evaluate_knight(int row, int col) {
    int score = 0;
    char piece = board[row][col];

    // More nuanced knight positioning
    int center_dist = abs(3 - col) + abs(3 - row);
    score += (6 - center_dist);  // Max bonus in center

    // Knights are better with pawns nearby for protection
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (is_within_bounds(row + dr, col + dc)) {
                char nearby = board[row + dr][col + dc];
                if (tolower(nearby) == 'p' && is_white_piece(nearby) == is_white_piece(piece)) {
                    score += 1;  // Bonus for nearby pawns
                }
            }
        }
    }
    return score;
}

int evaluate_bishop(int row, int col) {
    int score = 0;
    char piece = board[row][col];

    // Enhanced bishop evaluation
    if ((row + col) % 2 == 0) {
        score += 2;  // Bonus for controlling light squares
    }

    // Count available diagonals
    int mobility = 0;
    for (int dr = -1; dr <= 1; dr += 2) {
        for (int dc = -1; dc <= 1; dc += 2) {
            int r = row + dr, c = col + dc;
            while (is_within_bounds(r, c) && board[r][c] == '.') {
                mobility++;
                r += dr;
                c += dc;
            }
        }
    }
    score += mobility / 2;  // Bonus for mobility
    return score;
}

int evaluate_rook(int row, int col) {
    int score = 0;
    char piece = board[row][col];

    // Enhanced rook evaluation
    int open_file = 1;
    int semi_open = 1;
    for (int r = 0; r < 8; r++) {
        if (r != row) {
            char piece_on_file = board[r][col];
            if (piece_on_file != '.') {
                open_file = 0;
                if (tolower(piece_on_file) == 'p' && is_white_piece(piece_on_file) == is_white_piece(piece)) {
                    semi_open = 0;
                }
            }
        }
    }
    if (open_file) score += 5;  // Bonus for open file
    else if (semi_open) score += 3;  // Bonus for semi-open file

    // Bonus for 7th rank (2nd for black)
    if ((is_white_piece(piece) && row == 1) || (!is_white_piece(piece) && row == 6)) {
        score += 3;
    }
    return score;
}

int evaluate_queen(int row, int col, int current_move_count) {
    int score = 0;
    char piece = board[row][col];

    const int queen_table[8][8] = {
        {-20,-10,-10, -5, -5,-10,-10,-20},
        {-10,-20,-20,-20,-20,-20,-20,-10},  // Discourage early queen moves
        {-10,-20,  0,  0,  0,  0,-20,-10},  // Discourage early queen moves
        { -5,  0,  5,  5,  5,  5,  0, -5},
        {  0,  0,  5,  5,  5,  5,  0, -5},
        {-10,  5,  5,  5,  5,  5,  0,-10},
        {-10,  0,  5,  0,  0,  0,  0,-10},
        {-20,-10,-10, -5, -5,-10,-10,-20}
    };

    // Queen positioning based on piece-square table
    score += queen_table[row][col];

    // Penalize early queen moves
    if (current_move_count < 15) {
        if (is_white_piece(piece)) {
            if (row != 7 || col != 3) {  // Queen moved
                score -= 1000;  // Strong penalty
            }
        } else {
            if (row != 0 || col != 3) {
                score -= 80;  // Penalty for black queen
            }
        }
    }
    return score;
}

int evaluate_king(int row, int col, int current_move_count) {
    int score = 0;
    char piece = board[row][col];

    // King evaluation based on game phase
    if (current_move_count < 20) {  // Opening/early middle game
        // Encourage castling position
        if (is_white_piece(piece)) {
            if (row == 7 && (col == 6 || col == 2)) score += 5;  // Castled
        } else {
            if (row == 0 && (col == 6 || col == 2)) score += 5;  // Castled
        }
    } else {  // Late middle game/endgame
        // Encourage king activity
        int center_dist = abs(3 - col) + abs(3 - row);
        score += (7 - center_dist) / 2;  // Bonus for being active
    }
    return score;
}

int can_attack(char piece, int from_row, int from_col, int to_row, int to_col, char board[8][8]) {
    // Check if the target position is within bounds
    if (!is_within_bounds(to_row, to_col)) {
        return 0; // Out of bounds
    }

    // Handle pawns separately due to unique attack pattern
    if (tolower(piece) == 'p') {
        int direction = is_white_piece(piece) ? -1 : 1; // White pawns attack upwards
        if ((to_row == from_row + direction) &&
            (to_col == from_col - 1 || to_col == from_col + 1) &&
            board[to_row][to_col] != '.' &&
            is_white_piece(board[to_row][to_col]) != is_white_piece(piece)) {
            return 1; // Pawn can attack the target position
        }
        return 0; // Pawns can't attack otherwise
    }

    // Offsets for knight moves
    if (tolower(piece) == 'n') {
        int knight_offsets[8][2] = {{-2, -1}, {-1, -2}, {1, -2}, {2, -1},
                                    {2, 1}, {1, 2}, {-1, 2}, {-2, 1}};
        for (int i = 0; i < 8; i++) {
            if (from_row + knight_offsets[i][0] == to_row &&
                from_col + knight_offsets[i][1] == to_col) {
                return 1; // Knight can attack the target position
            }
        }
        return 0;
    }

    // Sliding pieces (bishop, rook, queen)
    if (tolower(piece) == 'b' || tolower(piece) == 'r' || tolower(piece) == 'q') {
        int directions[8][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1},   // Rook directions
                                {-1, -1}, {1, 1}, {1, -1}, {-1, 1}}; // Bishop directions
        int limit = (tolower(piece) == 'r') ? 4 : (tolower(piece) == 'b') ? 4 : 8;

        for (int d = 0; d < limit; d++) {
            int current_row = from_row + directions[d][0];
            int current_col = from_col + directions[d][1];
            while (is_within_bounds(current_row, current_col)) {
                if (current_row == to_row && current_col == to_col &&
                    is_white_piece(board[current_row][current_col]) != is_white_piece(piece)) {
                    return 1; // Can attack the target position
                }
                if (board[current_row][current_col] != '.') break; // Blocked by a piece
                current_row += directions[d][0];
                current_col += directions[d][1];
            }
        }
        return 0;
    }

    // King can attack adjacent squares
    if (tolower(piece) == 'k') {
        for (int dr = -1; dr <= 1; dr++) {
            for (int dc = -1; dc <= 1; dc++) {
                if (dr == 0 && dc == 0) continue;
                if (from_row + dr == to_row && from_col + dc == to_col) {
                    return 1; // King can attack the target position
                }
            }
        }
        return 0;
    }

    return 0; // Unknown piece or invalid attack
}


int evaluate_position(char piece, int row, int col, int current_move_count, char board[8][8]) {
    int position_value = 0;

    int capture_penalty = 0;
    for (int r = max(0, row - 2); r <= min(7, row + 2); r++) {
        for (int c = max(0, col - 2); c <= min(7, col + 2); c++) {
            char target_piece = board[r][c];
            if (is_white_piece(target_piece) != is_white_piece(piece) && target_piece != '.') {
                // Check if the opponent's piece can directly attack the square
                if (can_attack(target_piece, r, c, row, col, board)) {
                    int attacker_value = evaluate_captures(target_piece); // Value of attacker
                    capture_penalty -= 500; // Scaled penalty
                }
            }
        }
    }
    capture_penalty = max(capture_penalty, -500);

    // Add the capture penalty to the position value
    position_value += capture_penalty;

    switch (tolower(piece)) {
        case 'p':
            position_value += evaluate_pawn(row, col, current_move_count);
            break;
        case 'n':
            position_value += evaluate_knight(row, col);
            break;
        case 'b':
            position_value += evaluate_bishop(row, col);
            break;
        case 'r':
            position_value += evaluate_rook(row, col);
            break;
        case 'q':
            position_value += evaluate_queen(row, col, current_move_count);
            break;
        case 'k':
            position_value += evaluate_king(row, col, current_move_count);
            break;
    }
    return position_value;
}

// Material values
const int PAWN_VALUE = 100;
const int KNIGHT_VALUE = 320;
const int BISHOP_VALUE = 330;
const int ROOK_VALUE = 500;
const int QUEEN_VALUE = 900;
const int KING_VALUE = 20000;


int evaluate_piece_captures(char board[8][8], int row, int col, char piece, int is_white) {
    int capture_score = 0;

    if (tolower(piece) == 'p') {
        // Pawns only capture diagonally
        int attack_row = is_white ? row - 1 : row + 1;
        for (int attack_col = col - 1; attack_col <= col + 1; attack_col += 2) {
            if (is_within_bounds(attack_row, attack_col) && board[attack_row][attack_col] != '.') {
                char target_piece = board[attack_row][attack_col];
                if (is_white_piece(target_piece) != is_white) {
                    capture_score += evaluate_captures(target_piece);
                }
            }
        }
    } else if (tolower(piece) == 'n') {
        // Knights' unique move offsets
        int knight_offsets[8][2] = {{-2, -1}, {-1, -2}, {1, -2}, {2, -1},
                                    {2, 1}, {1, 2}, {-1, 2}, {-2, 1}};
        for (int k = 0; k < 8; k++) {
            int attack_row = row + knight_offsets[k][0];
            int attack_col = col + knight_offsets[k][1];
            if (is_within_bounds(attack_row, attack_col) && board[attack_row][attack_col] != '.') {
                char target_piece = board[attack_row][attack_col];
                if (is_white_piece(target_piece) != is_white) {
                    capture_score += evaluate_captures(target_piece);
                }
            }
        }
    } else if (tolower(piece) == 'b' || tolower(piece) == 'r' || tolower(piece) == 'q') {
        // Sliding pieces (bishops, rooks, queens)
        int directions[8][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1},   // Rook directions
                                {-1, -1}, {1, 1}, {1, -1}, {-1, 1}}; // Bishop directions
        int limit = (tolower(piece) == 'r') ? 4 : (tolower(piece) == 'b') ? 4 : 8;

        for (int d = 0; d < limit; d++) {
            int attack_row = row + directions[d][0];
            int attack_col = col + directions[d][1];
            while (is_within_bounds(attack_row, attack_col)) {
                char target_piece = board[attack_row][attack_col];
                if (target_piece != '.') {
                    if (is_white_piece(target_piece) != is_white) {
                        capture_score += evaluate_captures(target_piece);
                    }
                    break;  // Stop sliding when hitting a piece
                }
                attack_row += directions[d][0];
                attack_col += directions[d][1];
            }
        }
    } else if (tolower(piece) == 'k') {
        // Kings capture adjacent squares
        for (int dr = -1; dr <= 1; dr++) {
            for (int dc = -1; dc <= 1; dc++) {
                if (dr == 0 && dc == 0) continue;
                int attack_row = row + dr;
                int attack_col = col + dc;
                if (is_within_bounds(attack_row, attack_col) && board[attack_row][attack_col] != '.') {
                    char target_piece = board[attack_row][attack_col];
                    if (is_white_piece(target_piece) != is_white) {
                        capture_score += evaluate_captures(target_piece);
                    }
                }
            }
        }
    }
    return capture_score;
}

int evaluate_board(char board[8][8], int current_move_count) {
    int total_score = 0;

    // Track development and center control
    int white_developed_pieces = 0;
    int black_developed_pieces = 0;
    int white_center_pawns = 0;
    int black_center_pawns = 0;

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            char piece = board[i][j];
            if (piece == '.') continue;  // Skip empty squares

            int piece_value = 0;
            int position_value = evaluate_position(piece, i, j, current_move_count, board);

            // Assign piece value based on type
            switch (tolower(piece)) {
                case 'p': piece_value = PAWN_VALUE; break;
                case 'n': piece_value = KNIGHT_VALUE; break;
                case 'b': piece_value = BISHOP_VALUE; break;
                case 'r': piece_value = ROOK_VALUE; break;
                case 'q': piece_value = QUEEN_VALUE; break;
                case 'k': piece_value = KING_VALUE; break;
            }

            // Update total score based on piece color
            if (is_white_piece(piece)) {
                total_score += piece_value + position_value;
                if (i < 7) white_developed_pieces++;  // Track development
                if (j >= 3 && j <= 4) white_center_pawns++;  // Track center pawns

                // // Evaluate captures for white
                total_score += evaluate_piece_captures(board, i, j, piece, 1);
            } else {
                total_score -= piece_value + position_value;
                if (i > 0) black_developed_pieces++;
                if (j >= 3 && j <= 4) black_center_pawns++;

                // // Evaluate captures for black
                total_score -= evaluate_piece_captures(board, i, j, piece, 0);
            }
        }
    }

    // // Adjust total score based on development and center control
    total_score += (white_developed_pieces - black_developed_pieces) * 5; // Increased multiplier
    total_score += (white_center_pawns - black_center_pawns) * 5; // Increased multiplier

    // Add king safety evaluation for both sides
    total_score += evaluate_king_safety(board, 1);  // White
    total_score += evaluate_king_safety(board, 0);  // Black

    return total_score;
}