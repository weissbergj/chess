#include "move_sort.h"
#include "move_scoring.h"
#include "strings.h"
#include "game_state.h"
#include "game_logic.h"
#include "move_generation.h"
#include "board.h"

// Evaluate castling
int evaluate_castling(char piece, int src_col, int dest_col) {
    return abs(dest_col - src_col) == 2 ? 5000 : -6000; // Bonus for castling, penalty otherwise
}

// Evaluate captures; used in score_move
int evaluate_captures(char captured) {
    switch (tolower(captured)) {
        case 'q': return 900;  // Queen value
        case 'r': return 500;  // Rook value
        case 'b': case 'n': return 300;  // Bishop and Knight value
        case 'p': return 100;  // Pawn value
        default: return 0;
    }
}

// Move sorting logic for prioritizing good moves
int move_sort(char board[8][8], int src_row, int src_col, int dest_row, int dest_col, int move_count) {
    int score = 0;
    char piece = board[src_row][src_col];
    char captured = board[dest_row][dest_col];

    // Early game heuristics
    if (move_count < 15) {
        switch (tolower(piece)) {
            case 'n': 
                score += 30;  // Encourage early knight development
                break;
            case 'b': 
                score += 20;  // Encourage bishop development
                break;
            case 'p': 
                score += evaluate_pawn(src_row, src_col, move_count);  // Pawn evaluation
                if (dest_col >= 3 && dest_col <= 4) {
                    score += 10;  // Bonus for central control
                }
                break;
            case 'k': 
                score += evaluate_castling(piece, src_col, dest_col);  // Castling evaluation
                break;
            case 'q': 
                score -= 2000;  // Penalize early queen movement
                break;
        }
    }

    // Captures
    if (captured != '.') {
        score += evaluate_captures(captured) + 50;  // Prioritize captures strongly
    }

    // Check and checkmate prioritization
    char temp_board[8][8];
    memcpy(temp_board, board, sizeof(temp_board));
    make_move(temp_board, src_row, src_col, dest_row, dest_col, 0);
    if (is_in_check(temp_board, !is_white_piece(piece))) {
        score += 500;  // Bonus for delivering check
        if (is_game_over(temp_board, NULL, !is_white_piece(piece))) {
            score += 10000;  // Huge bonus for delivering checkmate
        }
    }

    // King safety considerations
    if (tolower(piece) == 'k') {
        int king_row, king_col;
        find_king(temp_board, &king_row, &king_col, is_white_piece(piece));
        if (is_open_file(board, king_col, is_white_piece(piece))) {
            score -= 50;  // Penalize moves leaving the king in open files
        }
    }

    // Positional bonuses
    if (tolower(piece) != 'k') {
        if (dest_col >= 3 && dest_col <= 4) {
            score += 20;  // Bonus for controlling central columns
        }
        if (tolower(piece) == 'n' && dest_row >= 2 && dest_row <= 5 && dest_col >= 2 && dest_col <= 5) {
            score += 25;  // Bonus for knights in central positions
        }
    }

    // Penalize redundant or passive pawn moves
    if (tolower(piece) == 'p' && src_col == dest_col) {
        score -= 5;  // Penalize non-central or passive pawn moves like a2-a3
    }

    return score;
}
