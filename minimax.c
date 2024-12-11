#include "minimax.h"
#include "move_generation.h"
#include "move_validation.h"
#include "board.h"
#include "move_scoring.h"
#include "game_logic.h"
#include "move_sort.h"
#include "utils.h"
#include "timer.h"
#include "printf.h"
#include "strings.h"

typedef struct {
    int move[4]; // The move represented as [src_row, src_col, dest_row, dest_col]
    int score;   // The score of the move
} ScoredMove;

typedef enum { false, true } bool;

// Function to check if two moves are equal
bool moves_are_equal(int move1[4], int move2[4]) {
    return (move1[0] == move2[0] && move1[1] == move2[1] &&
            move1[2] == move2[2] && move1[3] == move2[3]);
}

// Function to check if a move already exists in the moves array
bool move_exists(int moves[][4], int move_count, int new_move[4]) {
    for (int i = 0; i < move_count; i++) {
        if (moves_are_equal(moves[i], new_move)) {
            return true; // Move already exists
        }
    }
    return false; // Move is unique
}

// Function to generate unique moves
void generate_unique_moves(char board[8][8], int unique_moves[][4], int *unique_move_count, int moves[][4], int move_count) {
    *unique_move_count = 0; // Reset unique move count
    for (int i = 0; i < move_count; i++) {
        if (!move_exists(unique_moves, *unique_move_count, moves[i])) {
            memcpy(unique_moves[*unique_move_count], moves[i], sizeof(int) * 4);
            (*unique_move_count)++;
        }
    }
}

// // Function to sort scored moves using insertion sort
// void sort_scored_moves(ScoredMove scored_moves[], int scored_count) {
//     for (int i = 1; i < scored_count; i++) {
//         ScoredMove key = scored_moves[i];
//         int j = i - 1;
//         while (j >= 0 && scored_moves[j].score < key.score) {
//             scored_moves[j + 1] = scored_moves[j];
//             j--;
//         }
//         scored_moves[j + 1] = key;
//     }
// }


// Function to sort scored moves using bubble sort
void sort_scored_moves(ScoredMove scored_moves[], int scored_count) {
    for (int i = 0; i < scored_count - 1; i++) {
        for (int j = 0; j < scored_count - i - 1; j++) {
            if (scored_moves[j].score < scored_moves[j + 1].score) {
                // Swap scored_moves[j] and scored_moves[j + 1]
                ScoredMove temp = scored_moves[j];
                scored_moves[j] = scored_moves[j + 1];
                scored_moves[j + 1] = temp;
            }
        }
    }
}

// Minimax with alpha-beta pruning
int minimax(char board[8][8], int depth, int alpha, int beta, int maximizing_player, int current_move_count) {
    g_positions_examined++;
    if (g_positions_examined > g_max_positions) {
        return evaluate_board(board, current_move_count);
    }

    int is_checkmate;
    if (is_game_over(board, &is_checkmate, maximizing_player)) {
        return is_checkmate ? (maximizing_player ? -20000 + depth : 20000 - depth) : 0;
    }
    
    if (depth == 0) {
        return evaluate_board(board, current_move_count);
    }

    int moves[200][4];
    int move_count = 0;

    // Generate moves for the correct color
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            char piece = board[i][j];
            if ((maximizing_player && is_white_piece(piece)) || (!maximizing_player && is_black_piece(piece))) {
                switch (tolower(piece)) {
                    case 'p': generate_pawn_moves(board, i, j, moves, &move_count); break;
                    case 'n': generate_knight_moves(board, i, j, moves, &move_count); break;
                    case 'b': generate_bishop_moves(board, i, j, moves, &move_count); break;
                    case 'r': generate_rook_moves(board, i, j, moves, &move_count); break;
                    case 'q': generate_queen_moves(board, i, j, moves, &move_count); break;
                    case 'k': generate_king_moves(board, i, j, moves, &move_count); break;
                }
            }
        }
    }

    // Score and store moves
    ScoredMove scored_moves[200]; // Declare scored_moves array
    int scored_count = 0; // Initialize scored_count

    for (int i = 0; i < move_count; i++) {
        memcpy(scored_moves[scored_count].move, moves[i], sizeof(int) * 4);
        int base_score = move_sort(board, moves[i][0], moves[i][1], moves[i][2], moves[i][3], current_move_count);
        scored_moves[scored_count].score = base_score;
        scored_count++;
    }

    // Sort moves by score
    sort_scored_moves(scored_moves, scored_count);

    // Alpha-beta pruning
    if (maximizing_player) {
        int max_eval = -20000;
        for (int i = 0; i < scored_count; i++) {
            char temp_board[8][8];
            memcpy(temp_board, board, sizeof(temp_board));
            make_move(temp_board, scored_moves[i].move[0], scored_moves[i].move[1], 
                            scored_moves[i].move[2], scored_moves[i].move[3], 0);
            int eval = minimax(temp_board, depth - 1, alpha, beta, 0, current_move_count + 1);
            max_eval = eval > max_eval ? eval : max_eval;
            alpha = eval > alpha ? eval : alpha; // Update alpha
        }
        return max_eval;
    } else {
        int min_eval = 20000;
        for (int i = 0; i < scored_count; i++) {
            char temp_board[8][8];
            memcpy(temp_board, board, sizeof(temp_board));
            make_move(temp_board, scored_moves[i].move[0], scored_moves[i].move[1], 
                      scored_moves[i].move[2], scored_moves[i].move[3], 0);
            int eval = minimax(temp_board, depth - 1, alpha, beta, 1, current_move_count + 1);
            min_eval = eval < min_eval ? eval : min_eval;
            beta = eval < beta ? eval : beta; // Update beta
        }
        return min_eval;
    }
}

// Update find_best_move to use minimax
void find_best_move(char board[8][8], int best_move[4], int is_white) {
    int moves[200][4];
    int move_count = 0;
    int best_score = is_white ? -10000 : 10000;
    int depth = 4;

    // Generate moves for the correct color
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            char piece = board[i][j];
            if ((is_white && is_white_piece(piece)) || (!is_white && is_black_piece(piece))) {
                switch (tolower(piece)) {
                    case 'p': generate_pawn_moves(board, i, j, moves, &move_count); break;
                    case 'n': generate_knight_moves(board, i, j, moves, &move_count); break;
                    case 'b': generate_bishop_moves(board, i, j, moves, &move_count); break;
                    case 'r': generate_rook_moves(board, i, j, moves, &move_count); break;
                    case 'q': generate_queen_moves(board, i, j, moves, &move_count); break;
                    case 'k': generate_king_moves(board, i, j, moves, &move_count); break;
                }
            }
        }
    }

    // Remove duplicates from the moves array
    int unique_moves[200][4];
    int unique_move_count = 0;

    generate_unique_moves(board, unique_moves, &unique_move_count, moves, move_count);

    // Score and store unique moves
    ScoredMove scored_moves[200]; // Declare scored_moves array
    int scored_count = 0; // Initialize scored_count

    for (int i = 0; i < unique_move_count; i++) {
        int score = move_sort(board, unique_moves[i][0], unique_moves[i][1], unique_moves[i][2], unique_moves[i][3], unique_move_count);
        // Store the move and score
        scored_moves[scored_count].move[0] = unique_moves[i][0];
        scored_moves[scored_count].move[1] = unique_moves[i][1];
        scored_moves[scored_count].move[2] = unique_moves[i][2];
        scored_moves[scored_count].move[3] = unique_moves[i][3];
        scored_moves[scored_count].score = score;
        scored_count++;
    }

    // Sort moves using insertion sort for better efficiency
    sort_scored_moves(scored_moves, scored_count);

    // Evaluate sorted moves
    for (int i = 0; i < scored_count; i++) {
        char temp_board[8][8];
        memcpy(temp_board, board, sizeof(temp_board));
        
        make_move(temp_board, scored_moves[i].move[0], scored_moves[i].move[1], 
                  scored_moves[i].move[2], scored_moves[i].move[3], 0);
        
        int score = minimax(temp_board, depth - 1, -10000, 10000, !is_white, unique_move_count);
        
        // Log the score of the move being evaluated
        printf("Move (%d, %d -> %d, %d) scored: %d\n", 
               scored_moves[i].move[0], scored_moves[i].move[1], 
               scored_moves[i].move[2], scored_moves[i].move[3], score);

        if ((is_white && score > best_score) || (!is_white && score < best_score)) {
            best_score = score;
            memcpy(best_move, scored_moves[i].move, sizeof(int) * 4);
        }
    }
    
    // If no moves found, set invalid move
    if (unique_move_count == 0) {
        best_move[0] = -1;
        best_move[1] = -1;
        best_move[2] = -1;
        best_move[3] = -1;
    }

    // Log the best move selected
    printf("Best move from find_best_move: %d %d %d %d with score: %d\n", 
           best_move[0], best_move[1], best_move[2], best_move[3], best_score);
}