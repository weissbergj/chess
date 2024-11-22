#include "minimax.h"
#include "move_generation.h"
#include "move_validation.h"
#include "board.h"
#include "move_scoring.h"
#include "game_logic.h"
#include "utils.h"
#include "timer.h"
#include "printf.h"


// TO DO: 
// try minimax new ipmlementation after reviewing evaluate board and position cuz rn only board called
// figure out if best move is actually played by printing it in find_best_move
// figure out if we even need evaluate position...
// Figure out why we have early development stuff in the minimax.... we do early development in like a million places..
// figure out current move_count vs move_count in minimax and find_best_move; might be breaking...



// Minimax with alpha-beta pruning
int minimax(char board[8][8], int depth, int alpha, int beta, int maximizing_player, int current_move_count) {
    // printf("current move count from minimax: %d", move_count); / okay yeah so idk what the heck this current_move count is but it is always 0
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
    
    typedef struct {
        int move[4];
        int score;
    } ScoredMove;
    ScoredMove scored_moves[200];

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

    // Score and store moves with early game development priority
    for (int i = 0; i < move_count; i++) {
        memcpy(scored_moves[i].move, moves[i], sizeof(int) * 4);
        int base_score = score_move(board, moves[i][0], moves[i][1], moves[i][2], moves[i][3], current_move_count);
        
        // Early game development priority
        if (current_move_count < 10) {
            char piece = board[moves[i][0]][moves[i][1]];
            if (tolower(piece) == 'n' || tolower(piece) == 'b') {
                if ((maximizing_player && moves[i][0] == 7) || (!maximizing_player && moves[i][0] == 0)) {
                    base_score += 2000;
                }
            }
            if (tolower(piece) == 'p' && (moves[i][1] < 2 || moves[i][1] > 5)) {
                base_score -= 1000;
            }
        }
        
        scored_moves[i].score = base_score;
    }

    // Add randomization for equal moves in early game
    if (current_move_count < 10) {
        unsigned int ticks = timer_get_ticks();
        for (int i = 0; i < move_count; i++) {
            scored_moves[i].score += (ticks + i * 37) % 100;
            if (i > 0 && ((ticks + i * 41) % 3 == 0)) {
                ScoredMove temp = scored_moves[i];
                scored_moves[i] = scored_moves[i - 1];
                scored_moves[i - 1] = temp;
            }
        }
    }

    // Sort moves by score using insertion sort for better efficiency
    for (int i = 1; i < move_count; i++) {
        ScoredMove key = scored_moves[i];
        int j = i - 1;
        while (j >= 0 && scored_moves[j].score < key.score) {
            scored_moves[j + 1] = scored_moves[j];
            j--;
        }
        scored_moves[j + 1] = key;
    }

    if (maximizing_player) {
        int max_eval = -20000;
        for (int i = 0; i < move_count; i++) {
            char temp_board[8][8];
            memcpy(temp_board, board, sizeof(temp_board));
            
            make_move(temp_board, scored_moves[i].move[0], scored_moves[i].move[1], 
                                scored_moves[i].move[2], scored_moves[i].move[3], 0);
            int eval = minimax(temp_board, depth - 1, alpha, beta, 0, current_move_count + 1);
            
            max_eval = eval > max_eval ? eval : max_eval;
            alpha = alpha > eval ? alpha : eval;
            if (beta <= alpha) break;
        }
        return max_eval;
    } else {
        int min_eval = 20000;
        for (int i = 0; i < move_count; i++) {
            char temp_board[8][8];
            memcpy(temp_board, board, sizeof(temp_board));
            
            make_move(temp_board, scored_moves[i].move[0], scored_moves[i].move[1], 
                                scored_moves[i].move[2], scored_moves[i].move[3], 0);
            int eval = minimax(temp_board, depth - 1, alpha, beta, 1, current_move_count + 1);
            min_eval = eval < min_eval ? eval : min_eval;
            beta = beta < eval ? beta : eval;
            if (beta <= alpha) break;
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

    typedef struct {
        int move[4];
        int score;
    } ScoredMove;
    
    ScoredMove scored_moves[200];
    
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

    // Score and store moves
    for (int i = 0; i < move_count; i++) {
        memcpy(scored_moves[i].move, moves[i], sizeof(int) * 4);
        scored_moves[i].score = score_move(board, moves[i][0], moves[i][1], moves[i][2], moves[i][3], move_count);
    }

    // Sort moves using insertion sort for better efficiency
    for (int i = 1; i < move_count; i++) {
        ScoredMove key = scored_moves[i];
        int j = i - 1;
        while (j >= 0 && scored_moves[j].score < key.score) {
            scored_moves[j + 1] = scored_moves[j];
            j--;
        }
        scored_moves[j + 1] = key;
    }

    // Evaluate sorted moves
    for (int i = 0; i < move_count; i++) {
        char temp_board[8][8];
        memcpy(temp_board, board, sizeof(temp_board));
        
        make_move(temp_board, scored_moves[i].move[0], scored_moves[i].move[1], 
                            scored_moves[i].move[2], scored_moves[i].move[3], 0);
        
        int score = minimax(temp_board, depth - 1, -10000, 10000, !is_white, move_count);
        
        if ((is_white && score > best_score) || (!is_white && score < best_score)) {
            best_score = score;
            memcpy(best_move, scored_moves[i].move, sizeof(int) * 4);
            printf("\nbest move from find_best_move: %ls\n", best_move);
        }
    }
    
    // If no moves found, set invalid move
    if (move_count == 0) {
        best_move[0] = -1;
        best_move[1] = -1;
        best_move[2] = -1;
        best_move[3] = -1;
    }
}