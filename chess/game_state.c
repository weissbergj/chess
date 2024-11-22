#include "move_validation.h"
#include "game_state.h"
#include "game_logic.h"
#include "AI_logic.h"
#include "move_generation.h"
#include "board.h"
#include "utils.h"
#include "strings.h"
#include "uart.h"

// Initialize global variables
char board[8][8] = {
    {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},
    {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
    {'.', '.', '.', '.', '.', '.', '.', '.'},
    {'.', '.', '.', '.', '.', '.', '.', '.'},
    {'.', '.', '.', '.', '.', '.', '.', '.'},
    {'.', '.', '.', '.', '.', '.', '.', '.'},
    {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
    {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}
};

char initial_board[8][8] = {
    {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},
    {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
    {'.', '.', '.', '.', '.', '.', '.', '.'},
    {'.', '.', '.', '.', '.', '.', '.', '.'},
    {'.', '.', '.', '.', '.', '.', '.', '.'},
    {'.', '.', '.', '.', '.', '.', '.', '.'},
    {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
    {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}
};

Move move_history[MAX_HISTORY];
int move_count = 0;
int white_king_moved = 0;
int black_king_moved = 0;
int white_rooks_moved[2] = {0, 0};
int black_rooks_moved[2] = {0, 0};
int moves_without_capture = 0;
Position position_history[MAX_POSITIONS];
int position_count = 0;
GameState game_state = PLAYING;
int g_positions_examined = 0;
int g_max_positions = 5000;

void reset_game(void) {
    // Reset board to initial position
    memcpy(board, initial_board, sizeof(board));
    
    // Reset game state
    move_count = 0;
    moves_without_capture = 0;
    white_king_moved = black_king_moved = 0;
    white_rooks_moved[0] = white_rooks_moved[1] = 0;
    black_rooks_moved[0] = black_rooks_moved[1] = 0;
    position_count = 0;  // Reset position history
    memset(position_history, 0, sizeof(position_history));
    game_state = PLAYING;
}

int record_position(char board[8][8], int is_white_turn, int current_move_count) {
    // Don't check for repetition in early game
    if (current_move_count < 10) {  // Increased from 4 to 10 for more stable opening play
        return 1;
    }
    
    int repetitions = 1;

    // Check for repeated positions in the history
    int start_idx = (position_count >= MAX_POSITIONS) ? 
                    position_count - MAX_POSITIONS : 0;
    
    for (int i = start_idx; i < position_count; i++) {
        int idx = i % MAX_POSITIONS;
        // Ensure positions are properly separated
        if (position_history[idx].move_count < current_move_count - 1) {
            // Check if positions match
            int match = 1;
            for (int r = 0; r < 8 && match; r++) {
                for (int c = 0; c < 8 && match; c++) {
                    if (board[r][c] != position_history[idx].board[r][c]) {
                        match = 0;
                    }
                }
            }
            // Check turn and increment repetitions if match
            if (match && position_history[idx].is_white_turn == is_white_turn) {
                repetitions++;
                if (repetitions >= 3) {
                    return repetitions;  // Exit early on threefold
                }
            }
        }
    }

    // Store current position in circular buffer
    int store_idx = position_count % MAX_POSITIONS;
    memcpy(position_history[store_idx].board, board, sizeof(char) * 8 * 8);
    position_history[store_idx].is_white_turn = is_white_turn;
    position_history[store_idx].move_count = current_move_count;
    position_count++;

    return repetitions;
}

int same_position(char board1[8][8], char board2[8][8]) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board1[i][j] != board2[i][j]) return 0;
        }
    }
    return 1;
}

int is_repetition(char board[8][8], int is_white_turn) {
    int count = 0;
    
    // Bounds check
    if (position_count > MAX_POSITIONS) {
        position_count = MAX_POSITIONS;
    }
    
    for (int p = 0; p < position_count; p++) {
        int matches = 1;
        for (int i = 0; i < 8 && matches; i++) {
            for (int j = 0; j < 8 && matches; j++) {
                if (board[i][j] != position_history[p].board[i][j]) {
                    matches = 0;
                }
            }
        }
        if (matches && position_history[p].is_white_turn == is_white_turn) {
            count++;
        }
    }
    
    return count >= 3;
}

int is_insufficient_material(char board[8][8]) {
    int white_pieces = 0, black_pieces = 0;
    int white_bishops = 0, black_bishops = 0;
    int white_knights = 0, black_knights = 0;
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            char piece = board[i][j];
            if (piece == '.') continue;
            
            if (is_white_piece(piece)) {
                white_pieces++;
                if (piece == 'B') white_bishops++;
                if (piece == 'N') white_knights++;
            } else {
                black_pieces++;
                if (piece == 'b') black_bishops++;
                if (piece == 'n') black_knights++;
            }
        }
    }
    
    // King vs King
    if (white_pieces == 1 && black_pieces == 1) return 1;
    
    // King and Bishop/Knight vs King
    if ((white_pieces == 2 && black_pieces == 1) ||
        (white_pieces == 1 && black_pieces == 2)) {
        if (white_bishops + white_knights <= 1 &&
            black_bishops + black_knights <= 1) return 1;
    }
    
    return 0;
}

int is_endgame(char board[8][8]) {
    int white_material = 0, black_material = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            char piece = board[i][j];
            if (piece == '.') continue;
            
            switch(tolower(piece)) {
                case 'q': 
                    if (is_white_piece(piece)) white_material += 900;
                    else black_material += 900;
                    break;
                case 'r':
                    if (is_white_piece(piece)) white_material += 500;
                    else black_material += 500;
                    break;
                case 'b':
                case 'n':
                    if (is_white_piece(piece)) white_material += 300;
                    else black_material += 300;
                    break;
            }
        }
    }
    return (white_material <= 1300 && black_material <= 1300);  // Queen + Bishop/Knight or less
}

int is_near_stalemate(char board[8][8], int is_white) {
    // First check immediate stalemate danger
    int moves[200][4];
    int move_count = 0;
    int opponent_moves = 0;
    int material_advantage = 0;
    
    // Count material difference and mobility
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            char piece = board[i][j];
            if (piece != '.') {
                // Calculate material advantage
                int value = 0;
                switch(tolower(piece)) {
                    case 'q': value = 900; break;
                    case 'r': value = 500; break;
                    case 'b':
                    case 'n': value = 300; break;
                    case 'p': value = 100; break;
                }
                material_advantage += is_white_piece(piece) ? value : -value;
                
                // Count opponent's available moves
                if (is_white ? is_black_piece(piece) : is_white_piece(piece)) {
                    switch (tolower(piece)) {
                        case 'p': generate_pawn_moves(board, i, j, moves, &opponent_moves); break;
                        case 'n': generate_knight_moves(board, i, j, moves, &opponent_moves); break;
                        case 'b': generate_bishop_moves(board, i, j, moves, &opponent_moves); break;
                        case 'r': generate_rook_moves(board, i, j, moves, &opponent_moves); break;
                        case 'q': generate_queen_moves(board, i, j, moves, &opponent_moves); break;
                        case 'k': generate_king_moves(board, i, j, moves, &opponent_moves); break;
                    }
                }
            }
        }
    }
    
    // Adjust material advantage based on whose turn it is
    if (!is_white) material_advantage = -material_advantage;
    
    // Generate our moves and check for stalemate danger
    move_count = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            char piece = board[i][j];
            if (is_white ? is_white_piece(piece) : is_black_piece(piece)) {
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
    
    // Check each move for immediate stalemate
    for (int i = 0; i < move_count; i++) {
        char temp_board[8][8];
        memcpy(temp_board, board, sizeof(temp_board));
        make_move(temp_board, moves[i][0], moves[i][1], moves[i][2], moves[i][3], 0);
        
        if (!has_legal_moves(temp_board, !is_white) && !is_in_check(temp_board, !is_white)) {
            return 1;  // Immediate stalemate danger
        }
    }
    
    // Consider position dangerous if:
    // 1. We have significant material advantage (>= 5 pawns worth)
    // 2. Opponent has very few moves (<= 3)
    // 3. Not many pieces left on board (endgame)
    int piece_count = count_pieces(board);
    if (material_advantage >= 500 && opponent_moves <= 3 && piece_count <= 10) {
        return 1;  // Position likely to lead to stalemate
    }
    
    return 0;
}

int is_open_file(char board[8][8], int col) {
    for (int row = 0; row < 8; row++) {
        if (tolower(board[row][col]) == 'p') return 0;
    }
    return 1;
}

int is_white_piece(char piece) {
    return piece >= 'A' && piece <= 'Z';
}

int is_black_piece(char piece) {
    return piece >= 'a' && piece <= 'z';
}

// Helper function to check if destination can be captured
int can_capture(char src_piece, char dest_piece) {
    if (dest_piece == '.') return 1;  // Moving to empty square
    return is_white_piece(src_piece) ? is_black_piece(dest_piece) : is_white_piece(dest_piece);
}