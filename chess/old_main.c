/* File: main.c
 * -------------
 * Sample main program.
 */
#include "assert.h"
#include "backtrace.h"
#include "ccu.h"
#include "console.h"
#include "de.h"
#include "disassemble.h"
#include "fb.h"
#include "font.h"
#include "gl.h"
#include "gpio.h"
#include "gpio_extra.h"
#include "gpio_interrupt.h"
#include "hdmi.h"
#include "hstimer.h"
#include "interrupts.h"
#include "keyboard.h"
#include "malloc.h"
#include "mango.h"
#include "memmap.h"
#include "mouse.h"
#include "printf.h"
#include "ps2.h"
#include "ps2_keys.h"
#include "rand.h"
#include "ringbuffer.h"
#include "shell.h"
#include "shell_commands.h"
#include "strings.h"
#include "symtab.h"
#include "timer.h"
#include "uart.h"


// All global declarations and definitions go here
typedef struct {
    int from_row;
    int from_col;
    int to_row;
    int to_col;
    char piece;
    char captured;
} Move;

// Global variables for game state
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

Move move_history[1000];  // Store up to 1000 moves
int move_count = 0;      // Track number of moves made
int white_king_moved = 0;  // Track if kings have moved (for castling)
int black_king_moved = 0;
int white_rooks_moved[2] = {0, 0};  // Track if rooks have moved [queenside, kingside]
int black_rooks_moved[2] = {0, 0};
int moves_without_capture = 0;  // For 50-move rule
#define MAX_HISTORY 1000

typedef struct Position {
    char board[8][8];
    int is_white_turn;  // Add this
    int move_count;     // Add this to track exact position in game
} Position;

#define MAX_POSITIONS 100  // Reduce size to ensure it fits in memory
Position position_history[MAX_POSITIONS];
int position_count = 0;

typedef enum {
    PLAYING,
    DRAW_BY_REPETITION,
    DRAW_BY_FIFTY_MOVE,
    CHECKMATE,
    STALEMATE
} GameState;

GameState game_state = PLAYING;

int g_positions_examined = 0;
int g_max_positions = 5000;  // Adjustable limit

// Function declarations
int abs(int x);
char tolower(char c);

// Board utility functions
void display_board(char board[8][8]);
int is_within_bounds(int row, int col);
int is_white_piece(char piece);
int is_black_piece(char piece);
int can_capture(char src_piece, char dest_piece);
int count_pieces(char board[8][8]);

// Move parsing and display
void parse_move(char *input, int *src_row, int *src_col, int *dest_row, int *dest_col);
void display_move(int src_row, int src_col, int dest_row, int dest_col);
int is_valid_input_format(char *input);
int is_open_file(char board[8][8], int col);

// Move validation functions
int is_valid_move(char board[8][8], int src_row, int src_col, int dest_row, int dest_col);
int is_valid_pawn_move(char board[8][8], int src_row, int src_col, int dest_row, int dest_col);
int is_valid_rook_move(char board[8][8], int src_row, int src_col, int dest_row, int dest_col);
int is_valid_knight_move(char board[8][8], int src_row, int src_col, int dest_row, int dest_col);
int is_valid_bishop_move(char board[8][8], int src_row, int src_col, int dest_row, int dest_col);
int is_valid_queen_move(char board[8][8], int src_row, int src_col, int dest_row, int dest_col);
int is_valid_king_move(char board[8][8], int src_row, int src_col, int dest_row, int dest_col);

// Special move validation
int is_valid_en_passant(char board[8][8], int src_row, int src_col, int dest_row, int dest_col);
int can_castle_kingside(char board[8][8], int is_white);
int can_castle_queenside(char board[8][8], int is_white);

// Check and game state functions
int find_king(char board[8][8], int *king_row, int *king_col, int white_king);
int is_square_attacked(char board[8][8], int row, int col, int by_white);
int is_in_check(char board[8][8], int white_player);
int move_causes_check(char board[8][8], int src_row, int src_col, int dest_row, int dest_col);
int has_legal_moves(char board[8][8], int white_player);
int is_game_over(char board[8][8], int *is_checkmate, int white_player);
int is_insufficient_material(char board[8][8]);
int is_endgame(char board[8][8]);
int is_near_stalemate(char board[8][8], int is_white);
int same_position(char board1[8][8], char board2[8][8]);
int is_repetition(char board[8][8], int is_white_turn);


// Move generation functions
void generate_sliding_moves(char board[8][8], int row, int col, int directions[][2], 
                          int num_directions, int moves[][4], int *move_count);
void generate_pawn_moves(char board[8][8], int row, int col, int moves[][4], int *move_count);
void generate_knight_moves(char board[8][8], int row, int col, int moves[][4], int *move_count);
void generate_bishop_moves(char board[8][8], int row, int col, int moves[][4], int *move_count);
void generate_rook_moves(char board[8][8], int row, int col, int moves[][4], int *move_count);
void generate_queen_moves(char board[8][8], int row, int col, int moves[][4], int *move_count);
void generate_king_moves(char board[8][8], int row, int col, int moves[][4], int *move_count);
int evaluate_king_safety(char board[8][8], int is_white);

// AI and evaluation functions
int evaluate_position(char piece, int row, int col);
int evaluate_board(char board[8][8], int current_move_count);
int minimax(char board[8][8], int depth, int alpha, int beta, int maximizing_player, int current_move_count);
void find_best_move(char board[8][8], int best_move[4], int is_white);
int score_move(char board[8][8], int src_row, int src_col, 
               int dest_row, int dest_col, int move_count);

// Core game functions
void make_move(char board[8][8], int src_row, int src_col, int dest_row, int dest_col, int is_real_move);
void main(void);

// Absolute value function
int abs(int x) {
    return x >= 0 ? x : -x;
}

// Convert character to lowercase
char tolower(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c + ('a' - 'A');
    }
    return c;
}

int max(int a, int b) {
    return (a > b) ? a : b;
}

int min(int a, int b) {
    return (a < b) ? a : b;
}

void display_board(char board[8][8]) {
    uart_putstring("\n  a b c d e f g h\n"); // Column headers
    for (int i = 0; i < 8; i++) {
        uart_putchar('8' - i); // Row numbers
        uart_putchar(' ');
        for (int j = 0; j < 8; j++) {
            uart_putchar(board[i][j]);
            uart_putchar(' ');
        }
        uart_putchar('\n');
    }
}

void parse_move(char *input, int *src_row, int *src_col, int *dest_row, int *dest_col) {
    *src_col = input[0] - 'a';       // Column 'a'-'h' -> 0-7
    *src_row = '8' - input[1];       // Row '8'-'1' -> 0-7
    *dest_col = input[2] - 'a';      // Column 'a'-'h' -> 0-7
    *dest_row = '8' - input[3];      // Row '8'-'1' -> 0-7
}

// Helper function to check if a move is within board bounds
int is_within_bounds(int row, int col) {
    return row >= 0 && row < 8 && col >= 0 && col < 8;
}

// Helper function to check if a piece is white
int is_white_piece(char piece) {
    return piece >= 'A' && piece <= 'Z';
}

// Helper function to check if a piece is black
int is_black_piece(char piece) {
    return piece >= 'a' && piece <= 'z';
}

// Helper function to check if destination can be captured
int can_capture(char src_piece, char dest_piece) {
    if (dest_piece == '.') return 1;  // Moving to empty square
    return is_white_piece(src_piece) ? is_black_piece(dest_piece) : is_white_piece(dest_piece);
}

int is_valid_pawn_move(char board[8][8], int src_row, int src_col, int dest_row, int dest_col) {
    char piece = board[src_row][src_col];
    int direction = is_white_piece(piece) ? -1 : 1;  // White moves up (-1), Black moves down (+1)
    int start_row = is_white_piece(piece) ? 6 : 1;   // Starting row for pawns
    
    // Normal one-square move
    if (src_col == dest_col && dest_row == src_row + direction) {
        return board[dest_row][dest_col] == '.';
    }
    
    // Initial two-square move
    if (src_col == dest_col && src_row == start_row && dest_row == src_row + 2 * direction) {
        return board[dest_row][dest_col] == '.' && 
               board[src_row + direction][src_col] == '.';
    }
    
    // Capture moves
    if (abs(src_col - dest_col) == 1 && dest_row == src_row + direction) {
        if (board[dest_row][dest_col] != '.') {
            return can_capture(piece, board[dest_row][dest_col]);
        }
    }
    
    return 0;
}

int is_valid_rook_move(char board[8][8], int src_row, int src_col, int dest_row, int dest_col) {
    // Rook must move either horizontally or vertically
    if (src_row != dest_row && src_col != dest_col) return 0;
    
    // Check path for obstacles
    int row_step = (dest_row > src_row) ? 1 : (dest_row < src_row) ? -1 : 0;
    int col_step = (dest_col > src_col) ? 1 : (dest_col < src_col) ? -1 : 0;
    
    int current_row = src_row + row_step;
    int current_col = src_col + col_step;
    
    while (current_row != dest_row || current_col != dest_col) {
        if (board[current_row][current_col] != '.') return 0;
        current_row += row_step;
        current_col += col_step;
    }
    
    return can_capture(board[src_row][src_col], board[dest_row][dest_col]);
}

int is_valid_knight_move(char board[8][8], int src_row, int src_col, int dest_row, int dest_col) {
    int row_diff = abs(dest_row - src_row);
    int col_diff = abs(dest_col - src_col);
    
    return (row_diff == 2 && col_diff == 1) || (row_diff == 1 && col_diff == 2);
}

int is_valid_bishop_move(char board[8][8], int src_row, int src_col, int dest_row, int dest_col) {
    // Bishop must move diagonally
    if (abs(dest_row - src_row) != abs(dest_col - src_col)) return 0;
    
    int row_step = (dest_row > src_row) ? 1 : -1;
    int col_step = (dest_col > src_col) ? 1 : -1;
    
    int current_row = src_row + row_step;
    int current_col = src_col + col_step;
    
    while (current_row != dest_row) {
        if (board[current_row][current_col] != '.') return 0;
        current_row += row_step;
        current_col += col_step;
    }
    
    return can_capture(board[src_row][src_col], board[dest_row][dest_col]);
}

int is_valid_queen_move(char board[8][8], int src_row, int src_col, int dest_row, int dest_col) {
    // Queen can move like a rook or bishop
    return is_valid_rook_move(board, src_row, src_col, dest_row, dest_col) ||
           is_valid_bishop_move(board, src_row, src_col, dest_row, dest_col);
}

int is_valid_king_move(char board[8][8], int src_row, int src_col, int dest_row, int dest_col) {
    // King can move one square in any direction
    return abs(dest_row - src_row) <= 1 && abs(dest_col - src_col) <= 1;
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

// Modify make_move to integrate position tracking
void make_move(char board[8][8], int src_row, int src_col, int dest_row, int dest_col, int is_real_move) {
    char piece = board[src_row][src_col];
    char captured = board[dest_row][dest_col];
    
    // Only update counters and history for real moves
    if (is_real_move) {
        // Reset counter on pawn move or capture
        if (tolower(piece) == 'p' || captured != '.') {
            moves_without_capture = 0;
        } else {
            moves_without_capture++;
        }
        
        // Store move in history
        move_history[move_count].from_row = src_row;
        move_history[move_count].from_col = src_col;
        move_history[move_count].to_row = dest_row;
        move_history[move_count].to_col = dest_col;
        move_history[move_count].piece = piece;
        move_history[move_count].captured = captured;
        
        // Update king moved status for castling
        if (tolower(piece) == 'k') {
            if (is_white_piece(piece)) white_king_moved = 1;
            else black_king_moved = 1;
        }
        
        // Update rook moved status
        if (tolower(piece) == 'r') {
            if (src_row == 7 && src_col == 0) white_rooks_moved[0] = 1;
            if (src_row == 7 && src_col == 7) white_rooks_moved[1] = 1;
            if (src_row == 0 && src_col == 0) black_rooks_moved[0] = 1;
            if (src_row == 0 && src_col == 7) black_rooks_moved[1] = 1;
        }
    }
    
    // Handle castling move (for both real and analysis moves)
    if (tolower(piece) == 'k' && abs(dest_col - src_col) == 2) {
        // Kingside castling
        if (dest_col > src_col) {
            board[src_row][5] = board[src_row][7];
            board[src_row][7] = '.';
        }
        // Queenside castling
        else {
            board[src_row][3] = board[src_row][0];
            board[src_row][0] = '.';
        }
    }
    
    // Handle en passant capture
    if (tolower(piece) == 'p' && src_col != dest_col && captured == '.') {
        int captured_row = src_row;
        board[captured_row][dest_col] = '.';
    }
    
    // Make the actual move
    board[dest_row][dest_col] = piece;
    board[src_row][src_col] = '.';
    
    // Handle pawn promotion
    if (tolower(piece) == 'p' && (dest_row == 0 || dest_row == 7)) {
        char choice = is_white_piece(piece) ? 'Q' : 'q';
        board[dest_row][dest_col] = choice;
    }
    
    // Only check game state for real moves
    if (is_real_move) {
        // Check for fifty move rule
        if (moves_without_capture >= 100) {
            game_state = DRAW_BY_FIFTY_MOVE;
            return;
        }
        
        // Record position and check for repetition
        int repetitions = record_position(board, !is_white_piece(piece), move_count);
        if (repetitions >= 3) {
            game_state = DRAW_BY_REPETITION;
            uart_putstring("\nDraw by threefold repetition!\n");
            return;
        }
        
        // Update move count after everything else
        move_count++;
    }
}

int evaluate_position(char piece, int row, int col) {
    int position_value = 0;
    
    switch (tolower(piece)) {
        case 'p': {
            // Enhance pawn structure evaluation
            if (col >= 2 && col <= 5) {
                position_value += 2;
            }
            if ((col == 3 || col == 4) && (row >= 2 && row <= 5)) {
                position_value += 3;
            }
            
            // Stronger advancement incentive in endgame
            int endgame = (move_count > 30);
            if (is_white_piece(piece)) {
                position_value += endgame ? (7 - row) : (7 - row) / 2;
            } else {
                position_value += endgame ? row : row / 2;
            }
            
            // Penalize doubled pawns
            int doubled = 0;
            for (int r = 0; r < 8; r++) {
                if (r != row && tolower(board[r][col]) == 'p') {
                    doubled = 1;
                    break;
                }
            }
            if (doubled) position_value -= 5;
            break;
        }
        case 'n': {
            // More nuanced knight positioning
            int center_dist = abs(3 - col) + abs(3 - row);
            position_value += (6 - center_dist);  // Max bonus in center
            
            // Knights are better with pawns nearby for protection
            for (int dr = -1; dr <= 1; dr++) {
                for (int dc = -1; dc <= 1; dc++) {
                    if (is_within_bounds(row + dr, col + dc)) {
                        char nearby = board[row + dr][col + dc];
                        if (tolower(nearby) == 'p' && 
                            is_white_piece(nearby) == is_white_piece(piece)) {
                            position_value += 1;
                        }
                    }
                }
            }
            break;
        }
        case 'b': {
            // Enhanced bishop evaluation
            if ((row + col) % 2 == 0) {
                position_value += 2;
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
            position_value += mobility / 2;
            break;
        }
        case 'r': {
            // Enhanced rook evaluation
            int open_file = 1;
            int semi_open = 1;
            for (int r = 0; r < 8; r++) {
                if (r != row) {
                    char piece_on_file = board[r][col];
                    if (piece_on_file != '.') {
                        open_file = 0;
                        if (tolower(piece_on_file) == 'p' && 
                            is_white_piece(piece_on_file) == is_white_piece(piece)) {
                            semi_open = 0;
                        }
                    }
                }
            }
            if (open_file) position_value += 5;
            else if (semi_open) position_value += 3;
            
            // Bonus for 7th rank (2nd for black)
            if ((is_white_piece(piece) && row == 1) || 
                (!is_white_piece(piece) && row == 6)) {
                position_value += 3;
            }
            break;
        }
        case 'k': {
            // Different evaluation based on game phase
            if (move_count < 20) {  // Opening/early middle game
                // Encourage castling position
                if (is_white_piece(piece)) {
                    if (row == 7 && (col == 6 || col == 2)) position_value += 5;
                } else {
                    if (row == 0 && (col == 6 || col == 2)) position_value += 5;
                }
            } else {  // Late middle game/endgame
                // Encourage king activity
                int center_dist = abs(3 - col) + abs(3 - row);
                position_value += (7 - center_dist) / 2;
            }
            break;
        }
    }
    
    return position_value;
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

// Helper function to count total pieces
int count_pieces(char board[8][8]) {
    int count = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board[i][j] != '.') count++;
        }
    }
    return count;
}

int evaluate_board(char board[8][8], int current_move_count) {
    int total_score = 0;
    
    // Material values
    const int PAWN_VALUE = 100;
    const int KNIGHT_VALUE = 320;
    const int BISHOP_VALUE = 330;
    const int ROOK_VALUE = 500;
    const int QUEEN_VALUE = 900;
    const int KING_VALUE = 20000;
    
    // Updated piece-square tables with stronger values
    const int pawn_table[8][8] = {
        { 0,  0,   0,   0,   0,   0,  0,  0},
        {50, 50,  50,  50,  50,  50, 50, 50},
        {10, 10, 100, 200, 200, 100, 10, 10},  // Much stronger center pawn bonus
        { 5,  5,  50, 100, 100,  50,  5,  5},
        { 0,  0,   0,  50,  50,   0,  0,  0},
        { 5, -5, -10,   0,   0, -10, -5,  5},
        {-5,-10,-500,-500,-500,-500,-10, -5},  // Heavy penalty for f2/f7
        { 0,  0,   0,   0,   0,   0,  0,  0}
    };
    
    const int knight_table[8][8] = {
        {-50,-40,-30,-30,-30,-30,-40,-50},
        {-40,-20,  0,  5,  5,  0,-20,-40},
        {-30, 25, 35, 40, 40, 35, 25,-30},
        {-30, 35,400,450,450,400, 35,-30},  // Huge bonus for f3/c3
        {-30, 25, 35, 40, 40, 35, 25,-30},
        {-30,  0, 10, 15, 15, 10,  0,-30},
        {-40,-20,  0,  5,  5,  0,-20,-40},
        {-2000,-2000,-2000,-2000,-2000,-2000,-2000,-2000}  // Extreme back rank penalty
    };
    
    const int bishop_table[8][8] = {
        {-20,-10,-10,-10,-10,-10,-10,-20},
        {-10,  0, 20, 20, 20, 20,  0,-10},
        {-10, 20, 25, 30, 30, 25, 20,-10},
        {-10, 25, 25,100,100, 25, 25,-10},  // Better center control
        {-10, 20, 30, 30, 30, 30, 20,-10},
        {-10, 30, 30, 30, 30, 30, 30,-10},
        {-10,  5,  0,  0,  0,  0,  5,-10},
        {-2000,-2000,-2000,-2000,-2000,-2000,-2000,-2000}  // Extreme back rank penalty
    };
    
    const int rook_table[8][8] = {
        { 0,  0,  0,  0,  0,  0,  0,  0},
        { 5, 10, 10, 10, 10, 10, 10,  5},
        {-5,  0,  0,  0,  0,  0,  0, -5},
        {-5,  0,  0,  0,  0,  0,  0, -5},
        {-5,  0,  0,  0,  0,  0,  0, -5},
        {-5,  0,  0,  0,  0,  0,  0, -5},
        {-5,  0,  0,  0,  0,  0,  0, -5},
        { 0,  0,  0,  5,  5,  0,  0,  0}
    };
    
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
    
    const int king_table_middlegame[8][8] = {
        {-30,-40,-40,-50,-50,-40,-40,-30},
        {-30,-40,-40,-50,-50,-40,-40,-30},
        {-50,-60,-60,-70,-70,-60,-60,-50},  // Increased penalties
        {-70,-80,-80,-90,-90,-80,-80,-70},  // Much worse in center
        {-70,-80,-80,-90,-90,-80,-80,-70},
        {-50,-60,-60,-70,-70,-60,-60,-50},
        {-10,-20,-20,-20,-20,-20,-20,-10},
        { 20, 30, 10,  0,  0, 10, 30, 20}   // Back rank is safe
    };
    
    // Track development and center control
    int white_developed_pieces = 0;
    int black_developed_pieces = 0;
    int white_center_pawns = 0;
    int black_center_pawns = 0;
    int white_queen_moved = 0;
    int black_queen_moved = 0;
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            char piece = board[i][j];
            if (piece == '.') continue;
            
            int piece_value = 0;
            int position_value = 0;
            int row = is_white_piece(piece) ? i : 7-i;
            
            switch(tolower(piece)) {
                case 'p':
                    piece_value = PAWN_VALUE;
                    position_value = pawn_table[row][j];
                    
                    if (current_move_count < 20) {
                        if (j == 3 || j == 4) {  // d or e pawn
                            if (is_white_piece(piece)) {
                                white_center_pawns++;
                                position_value += 500;  // Much stronger center bonus
                            } else {
                                black_center_pawns++;
                                position_value += 500;
                            }
                        }
                        // Stronger penalty for wing pawns
                        if ((j <= 1 || j >= 6) && 
                            ((is_white_piece(piece) && i != 6) || 
                             (!is_white_piece(piece) && i != 1))) {
                            position_value -= 400;
                        }
                    }
                    break;
                    
                case 'n':
                    piece_value = KNIGHT_VALUE;
                    position_value = knight_table[row][j];
                    
                    if (current_move_count < 20) {
                        if (is_white_piece(piece)) {
                            if (i != 7) {  // Knight has moved
                                white_developed_pieces++;
                                if ((j >= 2 && j <= 5) && (i >= 2 && i <= 5)) {
                                    position_value += 300;  // Better center control
                                }
                                // Huge bonus for f3/c3
                                if ((i == 5 && (j == 2 || j == 5))) {
                                    position_value += 1000;
                                }
                            } else {
                                position_value -= 400;  // Stronger back rank penalty
                                if (j == 6) position_value -= 1000;  // Blocking castle
                            }
                        } else {
                            if (i != 0) {  // Knight has moved from back rank
                                black_developed_pieces++;
                                if ((j >= 2 && j <= 5) && (i >= 2 && i <= 5)) {
                                    position_value += 30;
                                }
                                // HUGE bonus for moving kingside knight
                                if (j == 6) {
                                    position_value += 200;
                                }
                            } else {
                                position_value -= 40;
                                // Extra penalty for blocking castle
                                if (j == 6) position_value -= 200;
                            }
                        }
                    }
                    break;

                case 'b':
                    piece_value = BISHOP_VALUE;
                    position_value = bishop_table[row][j];
                    
                    // Development tracking for bishops
                    if (current_move_count < 20) {
                        if (is_white_piece(piece)) {
                            if (i != 7) {  // Bishop has moved from back rank
                                white_developed_pieces++;
                                // Extra bonus for controlling center squares
                                if ((j >= 2 && j <= 5) && (i >= 2 && i <= 5)) {
                                    position_value += 30;
                                }
                                // HUGE bonus for moving kingside bishop
                                if (j == 5) {
                                    position_value += 200;
                                }
                            } else {
                                position_value -= 40;  // Penalty for staying on back rank
                                // Extra penalty for blocking castle
                                if (j == 5) position_value -= 200;
                            }
                        } else {
                            if (i != 0) {  // Bishop has moved from back rank
                                black_developed_pieces++;
                                if ((j >= 2 && j <= 5) && (i >= 2 && i <= 5)) {
                                    position_value += 30;
                                }
                                // HUGE bonus for moving kingside bishop
                                if (j == 5) {
                                    position_value += 200;
                                }
                            } else {
                                position_value -= 40;
                                // Extra penalty for blocking castle
                                if (j == 5) position_value -= 200;
                            }
                        }
                    }
                    break;
                    
                case 'r':
                    piece_value = ROOK_VALUE;
                    position_value = rook_table[row][j];
                    
                    // Encourage rook activation
                    if (current_move_count > 10) {  // After opening
                        if (is_open_file(board, j)) {
                            position_value += 30;  // Increased bonus
                        }
                        if ((is_white_piece(piece) && i == 1) || 
                            (!is_white_piece(piece) && i == 6)) {
                            position_value += 25;  // Bonus for 7th rank
                        }
                    }
                    break;
                    
                case 'q':
                    piece_value = QUEEN_VALUE;
                    position_value = queen_table[row][j];
                    
                    if (current_move_count < 15) {
                        if (is_white_piece(piece)) {
                            if (i != 7 || j != 3) {  // Queen moved
                                position_value -= 1000;  // Much stronger penalty
                                if (white_developed_pieces < 2) {
                                    position_value -= 2000;  // Extra penalty if undeveloped
                                }
                            }
                        } else {
                            if (i != 0 || j != 3) {
                                position_value += -80;
                                if (black_developed_pieces < 2) {
                                    position_value += -80;
                                }
                            }
                        }
                    }
                    break;
                    
                case 'k':
                    piece_value = KING_VALUE;

                    if (!is_endgame(board)) {
                        if (is_white_piece(piece)) {
                            position_value = king_table_middlegame[i][j];
                        } else {
                            position_value = king_table_middlegame[7-i][j];
                        }
                        
                        int start_rank = is_white_piece(piece) ? 7 : 0;
                        int start_file = 4;
                        
                        // Check for successful castling first
                        if (current_move_count < 30) {  // Early/mid game castling check
                            if (is_white_piece(piece)) {
                                if (i == 7 && (j == 6 || j == 2)) {  // Castled
                                    position_value += 10000;  // Huge castle bonus
                                } else if (i == 7 && j == 4 && current_move_count > 8) {
                                    position_value -= 2000;  // Stronger penalty for not castling
                                }
                            } else {
                                if (i == 0 && (j == 6 || j == 2)) {  // Kingside or Queenside castle
                                    position_value += 5000;  // HUGE bonus for castling
                                    // printf("\n!!! BLACK CASTLED !!!\n");
                                } else if (i == 0 && j == 4 && current_move_count > 8) {
                                    position_value -= 1000;  // Penalty for not castling when possible
                                }
                            }
                        }

                        // Your existing penalties for unsafe king moves
                        if (i != start_rank || j != start_file) {
                            // Only print for move counts 0-10, and only once per count
                            static int last_printed_move = -1;
                            if (current_move_count <= 10 && current_move_count != last_printed_move) {
                                last_printed_move = current_move_count;
                            }
                            
                            position_value -= 5000;  // Base penalty
                            if (j >= 2 && j <= 5) {
                                position_value -= 2000;
                            }
                            if (i >= 2 && i <= 5) {
                                position_value -= 2000;
                            }
                            if (current_move_count < 20) {
                                position_value -= 5000;
                            }
                        }
                    }
                    break;
            }
            
            if (is_white_piece(piece)) {
                total_score += piece_value + position_value;
            } else {
                total_score -= piece_value + position_value;
            }
        }
    }
    
    // Increase development bonuses in opening
    if (current_move_count < 10) {
        total_score += (white_developed_pieces - black_developed_pieces) * 40;  // Increased from 30
        total_score += (white_center_pawns - black_center_pawns) * 50;  // Increased from 40
        
        if (white_queen_moved) total_score -= 60;  // Increased from 40
        if (black_queen_moved) total_score += 60;
    }
    
    // Add king safety evaluation for both sides
    total_score += evaluate_king_safety(board, 1);  // White
    total_score += evaluate_king_safety(board, 0);  // Black
    
    // Opening principles (first 10 moves)
    int opening_bonus = 0;
    if (current_move_count < 20) {  // Both players' first 10 moves
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                char piece = board[i][j];
                if (piece == '.') continue;
                
                // Center control bonus for pawns and knights
                if ((j == 3 || j == 4) && (i == 3 || i == 4)) {
                    if (tolower(piece) == 'p' || tolower(piece) == 'n') {
                        opening_bonus += is_white_piece(piece) ? 30 : -30;
                    }
                }
                
                // Development bonus for minor pieces
                if (tolower(piece) == 'n' || tolower(piece) == 'b') {
                    int start_rank = is_white_piece(piece) ? 7 : 0;
                    if (i != start_rank) {
                        opening_bonus += is_white_piece(piece) ? 20 : -20;
                    }
                }
                
                // Penalize early queen moves
                if (tolower(piece) == 'q') {
                    int start_rank = is_white_piece(piece) ? 7 : 0;
                    if (i != start_rank) {
                        opening_bonus += is_white_piece(piece) ? -40 : 40;
                    }
                }
                
                // Castle bonus
                if (tolower(piece) == 'k') {
                    if (is_white_piece(piece) && i == 7 && (j == 1 || j == 6)) {
                        opening_bonus += 50;
                    }
                    if (!is_white_piece(piece) && i == 0 && (j == 1 || j == 6)) {
                        opening_bonus -= 50;
                    }
                }
            }
        }
        total_score += opening_bonus;
    }
    
    // Add specific endgame scoring
    if (is_endgame(board)) {
        // Encourage pushing enemy king to the edge
        int enemy_king_row, enemy_king_col;
        find_king(board, &enemy_king_row, &enemy_king_col, !is_white_piece(board[0][0]));
        
        // Distance from center penalty (force king to edge)
        int center_dist = abs(3.5 - enemy_king_row) + abs(3.5 - enemy_king_col);
        total_score += is_white_piece(board[0][0]) ? (center_dist * 10) : -(center_dist * 10);
        
        // Distance between kings bonus (keep kings apart)
        int friendly_king_row, friendly_king_col;
        find_king(board, &friendly_king_row, &friendly_king_col, is_white_piece(board[0][0]));
        int king_distance = abs(friendly_king_row - enemy_king_row) + 
                           abs(friendly_king_col - enemy_king_col);
        total_score += is_white_piece(board[0][0]) ? (king_distance * 5) : -(king_distance * 5);
        
        // Heavy penalty for stalemate positions
        if (!has_legal_moves(board, !is_white_piece(board[0][0])) && !is_in_check(board, !is_white_piece(board[0][0]))) {
            total_score += is_white_piece(board[0][0]) ? -1000 : 1000;  // Heavily penalize stalemate
        }

        // Add heavy penalty for positions that could lead to stalemate
        if (is_near_stalemate(board, is_white_piece(board[0][0]))) {
            total_score += is_white_piece(board[0][0]) ? -800 : 800;  // Slightly less than actual stalemate
        }
    }
    
    return is_white_piece(board[0][0]) ? total_score : -total_score;
}

// Helper function to check for open files
int is_open_file(char board[8][8], int col) {
    for (int row = 0; row < 8; row++) {
        if (tolower(board[row][col]) == 'p') return 0;
    }
    return 1;
}

void generate_knight_moves(char board[8][8], int row, int col, int moves[][4], int *move_count) {
    // All possible knight moves
    int knight_moves[8][2] = {
        {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
        {1, -2}, {1, 2}, {2, -1}, {2, 1}
    };
    
    char piece = board[row][col];
    
    for (int i = 0; i < 8; i++) {
        int new_row = row + knight_moves[i][0];
        int new_col = col + knight_moves[i][1];
        
        if (is_within_bounds(new_row, new_col)) {
            if (board[new_row][new_col] == '.' || 
                (is_black_piece(piece) ? is_white_piece(board[new_row][new_col]) 
                                     : is_black_piece(board[new_row][new_col]))) {
                moves[*move_count][0] = row;
                moves[*move_count][1] = col;
                moves[*move_count][2] = new_row;
                moves[*move_count][3] = new_col;
                (*move_count)++;
            }
        }
    }
}

void generate_pawn_moves(char board[8][8], int row, int col, int moves[][4], int *move_count) {
    char piece = board[row][col];
    int direction = is_white_piece(piece) ? -1 : 1;
    int start_row = is_white_piece(piece) ? 6 : 1;
    
    // Forward move
    if (is_within_bounds(row + direction, col) && board[row + direction][col] == '.') {
        moves[*move_count][0] = row;
        moves[*move_count][1] = col;
        moves[*move_count][2] = row + direction;
        moves[*move_count][3] = col;
        (*move_count)++;
        
        // Initial two-square move
        if (row == start_row && board[row + 2 * direction][col] == '.') {
            moves[*move_count][0] = row;
            moves[*move_count][1] = col;
            moves[*move_count][2] = row + 2 * direction;
            moves[*move_count][3] = col;
            (*move_count)++;
        }
    }
    
    // Captures
    for (int dc = -1; dc <= 1; dc += 2) {
        if (is_within_bounds(row + direction, col + dc)) {
            char target = board[row + direction][col + dc];
            if (target != '.' && 
                (is_black_piece(piece) ? is_white_piece(target) : is_black_piece(target))) {
                moves[*move_count][0] = row;
                moves[*move_count][1] = col;
                moves[*move_count][2] = row + direction;
                moves[*move_count][3] = col + dc;
                (*move_count)++;
            }
        }
    }
}

void generate_sliding_moves(char board[8][8], int row, int col, int directions[][2], 
                          int num_directions, int moves[][4], int *move_count) {
    char piece = board[row][col];
    
    for (int d = 0; d < num_directions; d++) {
        int dr = directions[d][0];
        int dc = directions[d][1];
        int new_row = row + dr;
        int new_col = col + dc;
        
        while (is_within_bounds(new_row, new_col)) {
            if (board[new_row][new_col] == '.') {
                moves[*move_count][0] = row;
                moves[*move_count][1] = col;
                moves[*move_count][2] = new_row;
                moves[*move_count][3] = new_col;
                (*move_count)++;
                // Debugging output
                printf("Generated move: %c%d to %c%d\n", 'a' + col, 8 - row, 'a' + new_col, 8 - new_row);
            } else {
                if ((is_black_piece(piece) && is_white_piece(board[new_row][new_col])) ||
                    (is_white_piece(piece) && is_black_piece(board[new_row][new_col]))) {
                    moves[*move_count][0] = row;
                    moves[*move_count][1] = col;
                    moves[*move_count][2] = new_row;
                    moves[*move_count][3] = new_col;
                    (*move_count)++;
                    // Debugging output
                    printf("Generated capture: %c%d to %c%d\n", 'a' + col, 8 - row, 'a' + new_col, 8 - new_row);
                }
                break;
            }
            new_row += dr;
            new_col += dc;
        }
    }
}

void generate_rook_moves(char board[8][8], int row, int col, int moves[][4], int *move_count) {
    int rook_directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    generate_sliding_moves(board, row, col, rook_directions, 4, moves, move_count);
}

void generate_bishop_moves(char board[8][8], int row, int col, int moves[][4], int *move_count) {
    int bishop_directions[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
    generate_sliding_moves(board, row, col, bishop_directions, 4, moves, move_count);
}

void generate_queen_moves(char board[8][8], int row, int col, int moves[][4], int *move_count) {
    generate_rook_moves(board, row, col, moves, move_count);
    generate_bishop_moves(board, row, col, moves, move_count);
}

int evaluate_king_safety(char board[8][8], int is_white) {
    int king_row, king_col;
    if (!find_king(board, &king_row, &king_col, is_white)) {
        return 0;  // Safety check in case king not found
    }
    
    int safety_score = 0;
    
    // Early/middlegame king safety (first 20 moves)
    if (move_count < 20) {
        // Severely penalize early king movement from back rank
        int start_rank = is_white ? 7 : 0;
        if (king_row != start_rank) {
            safety_score -= 100;  // Increased penalty for early king movement
        }
        
        // Pawn shield - more important in early/middlegame
        int pawn_rank = is_white ? king_row - 1 : king_row + 1;
        if (pawn_rank >= 0 && pawn_rank < 8) {
            for (int col = max(0, king_col - 1); col <= min(7, king_col + 1); col++) {
                if (board[pawn_rank][col] == (is_white ? 'P' : 'p')) {
                    safety_score += 30;  // Increased bonus for pawn shield
                }
            }
        }
    } else {
        // Late game - less severe penalties
        // Pawn shield still matters but less important
        int pawn_rank = is_white ? king_row - 1 : king_row + 1;
        if (pawn_rank >= 0 && pawn_rank < 8) {
            for (int col = max(0, king_col - 1); col <= min(7, king_col + 1); col++) {
                if (board[pawn_rank][col] == (is_white ? 'P' : 'p')) {
                    safety_score += 10;
                }
            }
        }
        
        // Still penalize open files but less severely
        for (int col = max(0, king_col - 1); col <= min(7, king_col + 1); col++) {
            if (is_open_file(board, col)) {
                safety_score -= 20;
            }
        }
    }
    
    return is_white ? safety_score : -safety_score;
}

void generate_king_moves(char board[8][8], int row, int col, int moves[][4], int *move_count) {
    char piece = board[row][col];
    int start_rank = is_white_piece(piece) ? 7 : 0;
    
    // Early game king move prevention
    if (row == start_rank && col == 4) {
        // Only generate moves if king is in check
        if (!is_in_check(board, is_white_piece(piece))) {
            return;  // Don't generate any moves if king is safe
        }
    }
    
    int king_directions[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        {0, -1},           {0, 1},
        {1, -1},  {1, 0},  {1, 1}
    };
    
    for (int i = 0; i < 8; i++) {
        int new_row = row + king_directions[i][0];
        int new_col = col + king_directions[i][1];
        
        if (is_within_bounds(new_row, new_col)) {
            if (board[new_row][new_col] == '.' || 
                (is_black_piece(piece) ? is_white_piece(board[new_row][new_col]) 
                                     : is_black_piece(board[new_row][new_col]))) {
                moves[*move_count][0] = row;
                moves[*move_count][1] = col;
                moves[*move_count][2] = new_row;
                moves[*move_count][3] = new_col;
                (*move_count)++;
            }
        }
    }
}

// Check if any legal moves exist for a player
int has_legal_moves(char board[8][8], int white_player) {
    // Try to find at least one legal move
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            char piece = board[i][j];
            if ((white_player && is_white_piece(piece)) || 
                (!white_player && is_black_piece(piece))) {
                // Try all possible destinations
                for (int di = 0; di < 8; di++) {
                    for (int dj = 0; dj < 8; dj++) {
                        if (is_valid_move(board, i, j, di, dj)) {
                            // Found a legal move
                            return 1;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

// Check for checkmate or stalemate
int is_game_over(char board[8][8], int *is_checkmate, int white_player) {
    // Check for king capture
    int white_king_found = 0;
    int black_king_found = 0;

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board[i][j] == 'K') {
                white_king_found = 1;
            }
            if (board[i][j] == 'k') {
                black_king_found = 1;
            }
        }
    }

    if (!white_king_found) {
        *is_checkmate = 1;  // Consider it a checkmate if the king is captured
        return 1;
    }

    if (!black_king_found) {
        *is_checkmate = 1;  // Consider it a checkmate if the king is captured
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

int score_move(char board[8][8], int src_row, int src_col, 
               int dest_row, int dest_col, int move_count) {
    int score = 0;
    char piece = board[src_row][src_col];
    char captured = board[dest_row][dest_col];
    
    // Early game focus (first 15 moves)
    if (move_count < 15) {
        // KNIGHTS FIRST - Highest priority
        if (tolower(piece) == 'n') {
            int home_rank = is_white_piece(piece) ? 7 : 0;
            if (src_row == home_rank) {  // Knight hasn't moved yet
                score += 8000;  // Huge bonus for developing knights
                // Extra bonus for f3/c3 or f6/c6
                if ((is_white_piece(piece) && dest_row == 5 && (dest_col == 2 || dest_col == 5)) ||  // c3/f3
                    (!is_white_piece(piece) && dest_row == 2 && (dest_col == 2 || dest_col == 5))) { // c6/f6
                    score += 4000;
                }
            }
        }

        // CENTER PAWNS SECOND
        if (tolower(piece) == 'p') {
            if ((src_col == 3 || src_col == 4) &&  // d or e pawn
                ((is_white_piece(piece) && src_row == 6 && dest_row == 4) ||  // d4/e4
                 (!is_white_piece(piece) && src_row == 1 && dest_row == 3))) {  // d5/e5
                score += 6000;  // Strong bonus for initial center pawn moves
            }
            // Discourage wing pawn moves
            if (src_col < 2 || src_col > 5) {  // a,b,g,h pawns
                score -= 5000;
            }
        }

        // CASTLING THIRD
        if (tolower(piece) == 'k') {
            if (abs(dest_col - src_col) == 2) {
                score += 5000;  // Huge bonus for castling
            } else {
                score -= 6000;  // Strong penalty for other king moves
            }
        }

        // DISCOURAGE EARLY QUEEN MOVES
        if (tolower(piece) == 'q') {
            score -= 6000;
        }
    }
    
    // Captures (lower priority in opening)
    if (captured != '.') {
        switch(tolower(captured)) {
            case 'q': score += 900; break;
            case 'r': score += 500; break;
            case 'b': case 'n': score += 300; break;
            case 'p': score += 100; break;
        }
    }
    
    // Standard positional bonuses
    char temp_board[8][8];
    memcpy(temp_board, board, sizeof(temp_board));
    make_move(temp_board, src_row, src_col, dest_row, dest_col, 0);
    
    if (is_in_check(temp_board, !is_white_piece(piece))) {
        score += 50;  // Small bonus for giving check
    }
    
    return score;
}

// Minimax with alpha-beta pruning
int minimax(char board[8][8], int depth, int alpha, int beta, int maximizing_player, int current_move_count) {
    g_positions_examined++;
    if (g_positions_examined > g_max_positions) {
        return evaluate_board(board, current_move_count);
    }

    int is_checkmate;
    if (is_game_over(board, &is_checkmate, maximizing_player)) {
        if (is_checkmate) {
            return maximizing_player ? -20000 + depth : 20000 - depth;
        }
        return 0;
    }
    
    if (depth == 0) {
        int eval = evaluate_board(board, current_move_count);
        return eval;
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
            if (maximizing_player ? is_white_piece(piece) : is_black_piece(piece)) {
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
        int base_score = score_move(board, 
                                  moves[i][0], moves[i][1],
                                  moves[i][2], moves[i][3],
                                  current_move_count);
                                  
        // Early game development priority
        if (current_move_count < 10) {
            char piece = board[moves[i][0]][moves[i][1]];
            if (tolower(piece) == 'n' || tolower(piece) == 'b') {
                if ((maximizing_player && moves[i][0] == 7) || 
                    (!maximizing_player && moves[i][0] == 0)) {
                    base_score += 2000;
                }
            }
            if (tolower(piece) == 'p' && 
                (moves[i][1] < 2 || moves[i][1] > 5)) {
                base_score -= 1000;
            }
        }
        
        scored_moves[i].score = base_score;
    }

    // Add randomization for equal moves in early game
    if (current_move_count < 10) {  // Only in early game
    unsigned int ticks = timer_get_ticks();
    for (int i = 0; i < move_count; i++) {
        // Add small random variation to all moves in opening
        scored_moves[i].score += (ticks + i * 37) % 100;  // More variation
        
        // Sometimes swap adjacent moves
        if (i > 0 && ((ticks + i * 41) % 3 == 0)) {
            ScoredMove temp = scored_moves[i];
            scored_moves[i] = scored_moves[i-1];
            scored_moves[i-1] = temp;
        }
    }
}

    // Sort moves by score
    for (int i = 0; i < move_count - 1; i++) {
        for (int j = 0; j < move_count - i - 1; j++) {
            if (scored_moves[j].score < scored_moves[j + 1].score) {
                ScoredMove temp = scored_moves[j];
                scored_moves[j] = scored_moves[j + 1];
                scored_moves[j + 1] = temp;
            }
        }
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
            if ((is_white && is_white_piece(piece)) ||
                (!is_white && is_black_piece(piece))) {
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
        scored_moves[i].score = score_move(board, 
                                         moves[i][0], moves[i][1],
                                         moves[i][2], moves[i][3],
                                         move_count);  // Just pass the existing move_count
    }

    // Sort moves (using insertion sort for better efficiency)
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
        
        if (is_white) {
            if (score > best_score) {
                best_score = score;
                memcpy(best_move, scored_moves[i].move, sizeof(int) * 4);
            }
        } else {
            if (score < best_score) {
                best_score = score;
                memcpy(best_move, scored_moves[i].move, sizeof(int) * 4);
            }
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

// Input validation
int is_valid_input_format(char *input) {
    if (strlen(input) != 4) return 0;
    
    // Check source square
    if (input[0] < 'a' || input[0] > 'h') return 0;
    if (input[1] < '1' || input[1] > '8') return 0;
    
    // Check destination square
    if (input[2] < 'a' || input[2] > 'h') return 0;
    if (input[3] < '1' || input[3] > '8') return 0;
    
    return 1;
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

// Check if a square is under attack by the opponent
int is_square_attacked(char board[8][8], int row, int col, int by_white) {
    // Check for attacks by pawns
    int pawn_direction = by_white ? 1 : -1;
    if (is_within_bounds(row + pawn_direction, col - 1)) {
        char piece = board[row + pawn_direction][col - 1];
        if ((by_white && piece == 'P') || (!by_white && piece == 'p')) {
            return 1;
        }
    }
    if (is_within_bounds(row + pawn_direction, col + 1)) {
        char piece = board[row + pawn_direction][col + 1];
        if ((by_white && piece == 'P') || (!by_white && piece == 'p')) {
            return 1;
        }
    }

    // Check for attacks by knights
    int knight_moves[8][2] = {
        {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
        {1, -2}, {1, 2}, {2, -1}, {2, 1}
    };
    for (int i = 0; i < 8; i++) {
        int new_row = row + knight_moves[i][0];
        int new_col = col + knight_moves[i][1];
        if (is_within_bounds(new_row, new_col)) {
            char piece = board[new_row][new_col];
            if ((by_white && piece == 'N') || (!by_white && piece == 'n')) {
                return 1;
            }
        }
    }

    // Check for attacks by sliding pieces (queen, rook, bishop)
    int directions[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        {0, -1},           {0, 1},
        {1, -1},  {1, 0},  {1, 1}
    };
    
    for (int d = 0; d < 8; d++) {
        int new_row = row + directions[d][0];
        int new_col = col + directions[d][1];
        int distance = 1;
        
        while (is_within_bounds(new_row, new_col)) {
            char piece = board[new_row][new_col];
            if (piece != '.') {
                if (by_white) {
                    if ((piece == 'Q') || 
                        (piece == 'R' && (directions[d][0] == 0 || directions[d][1] == 0)) ||
                        (piece == 'B' && (directions[d][0] != 0 && directions[d][1] != 0))) {
                        return 1;
                    }
                } else {
                    if ((piece == 'q') || 
                        (piece == 'r' && (directions[d][0] == 0 || directions[d][1] == 0)) ||
                        (piece == 'b' && (directions[d][0] != 0 && directions[d][1] != 0))) {
                        return 1;
                    }
                }
                break;
            }
            new_row += directions[d][0];
            new_col += directions[d][1];
            distance++;
        }
    }

    // Check for attacks by king
    for (int d = 0; d < 8; d++) {
        int new_row = row + directions[d][0];
        int new_col = col + directions[d][1];
        if (is_within_bounds(new_row, new_col)) {
            char piece = board[new_row][new_col];
            if ((by_white && piece == 'K') || (!by_white && piece == 'k')) {
                return 1;
            }
        }
    }

    return 0;
}

// Find the king's position
int find_king(char board[8][8], int *king_row, int *king_col, int is_white) {
    char king = is_white ? 'K' : 'k';
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board[i][j] == king) {
                *king_row = i;
                *king_col = j;
                return 1;  // Found the king
            }
        }
    }
    return 0;  // King not found (shouldn't happen in valid position)
}

// Check if a player is in check
int is_in_check(char board[8][8], int white_player) {
    int king_row, king_col;
    find_king(board, &king_row, &king_col, white_player);
    return is_square_attacked(board, king_row, king_col, !white_player);
}

// Check if a move would leave the player in check
int move_causes_check(char board[8][8], int src_row, int src_col, int dest_row, int dest_col) {
    char temp_board[8][8];
    memcpy(temp_board, board, sizeof(char) * 8 * 8);
    
    // Make the move on the temporary board
    char piece = temp_board[src_row][src_col];
    temp_board[dest_row][dest_col] = piece;
    temp_board[src_row][src_col] = '.';
    
    // Check if the move leaves the player's king in check
    return is_in_check(temp_board, is_white_piece(piece));
}

// Update is_valid_move to include check validation
int is_valid_move(char board[8][8], int src_row, int src_col, int dest_row, int dest_col) {
    // Basic boundary checks
    if (!is_within_bounds(src_row, src_col) || !is_within_bounds(dest_row, dest_col)) {
        return 0;
    }
    
    char piece = board[src_row][src_col];
    if (piece == '.') {
        return 0;  // No piece at source
    }
    
    // Can't capture your own piece
    if (board[dest_row][dest_col] != '.' && 
        (is_white_piece(piece) == is_white_piece(board[dest_row][dest_col]))) {
        return 0;
    }
    
    // Check piece-specific rules
    int valid = 0;
    switch (tolower(piece)) {
        case 'p': {
            // Normal pawn move validation
            valid = is_valid_pawn_move(board, src_row, src_col, dest_row, dest_col);
            // Check en passant
            if (!valid && src_col != dest_col && board[dest_row][dest_col] == '.') {
                valid = is_valid_en_passant(board, src_row, src_col, dest_row, dest_col);
            }
            break;
        }
        case 'r': valid = is_valid_rook_move(board, src_row, src_col, dest_row, dest_col); break;
        case 'n': valid = is_valid_knight_move(board, src_row, src_col, dest_row, dest_col); break;
        case 'b': valid = is_valid_bishop_move(board, src_row, src_col, dest_row, dest_col); break;
        case 'q': valid = is_valid_queen_move(board, src_row, src_col, dest_row, dest_col); break;
        case 'k': {
            if (abs(dest_col - src_col) == 2) {
                // Castling moves are handled by separate functions
                valid = (dest_col > src_col) ? 
                    can_castle_kingside(board, is_white_piece(piece)) :
                    can_castle_queenside(board, is_white_piece(piece));
            } else {
                valid = is_valid_king_move(board, src_row, src_col, dest_row, dest_col);
            }
            break;
        }
        default: return 0;
    }
    
    if (!valid) return 0;
    
    // Check if move would leave player in check
    char temp_board[8][8];
    memcpy(temp_board, board, sizeof(temp_board));
    
    // Make the move on temporary board
    temp_board[dest_row][dest_col] = piece;
    temp_board[src_row][src_col] = '.';
    
    // Handle en passant capture on temporary board
    if (tolower(piece) == 'p' && src_col != dest_col && board[dest_row][dest_col] == '.') {
        temp_board[src_row][dest_col] = '.';  // Remove captured pawn
    }
    
    // Check if the move leaves the player's king in check
    return !is_in_check(temp_board, is_white_piece(piece));
}

void display_move(int src_row, int src_col, int dest_row, int dest_col) {
    char piece = board[src_row][src_col];
    
    // Check if it's a castling move
    if (tolower(piece) == 'k' && abs(dest_col - src_col) == 2) {
        if (dest_col > src_col) {
            uart_putstring("O-O");  // Kingside castling
        } else {
            uart_putstring("O-O-O");  // Queenside castling
        }
    } else {
        // Display normal move
        uart_putchar('a' + src_col);
        uart_putchar('8' - src_row);
        uart_putchar('a' + dest_col);
        uart_putchar('8' - dest_row);
    }
}

// Castling and en passant validation
int can_castle_kingside(char board[8][8], int is_white) {
    int row = is_white ? 7 : 0;
    char king = is_white ? 'K' : 'k';
    char rook = is_white ? 'R' : 'r';
    
    // Check if king and rook are in correct positions
    if (board[row][4] != king || board[row][7] != rook) return 0;
    
    // Check if king or rook has moved
    if (is_white ? white_king_moved : black_king_moved) return 0;
    if (is_white ? white_rooks_moved[1] : black_rooks_moved[1]) return 0;
    
    // Check if path is clear
    if (board[row][5] != '.' || board[row][6] != '.') return 0;
    
    // Check if king passes through check
    for (int col = 4; col <= 6; col++) {
        if (is_square_attacked(board, row, col, !is_white)) return 0;
    }
    
    return 1;
}

int can_castle_queenside(char board[8][8], int is_white) {
    int row = is_white ? 7 : 0;
    char king = is_white ? 'K' : 'k';
    char rook = is_white ? 'R' : 'r';
    
    // Check if king and rook are in correct positions
    if (board[row][4] != king || board[row][0] != rook) return 0;
    
    // Check if king or rook has moved
    if (is_white ? white_king_moved : black_king_moved) return 0;
    if (is_white ? white_rooks_moved[0] : black_rooks_moved[0]) return 0;
    
    // Check if path is clear
    if (board[row][1] != '.' || board[row][2] != '.' || board[row][3] != '.') return 0;
    
    // Check if king passes through check
    for (int col = 2; col <= 4; col++) {
        if (is_square_attacked(board, row, col, !is_white)) return 0;
    }
    
    return 1;
}
int is_valid_en_passant(char board[8][8], int src_row, int src_col, int dest_row, int dest_col) {
    if (move_count == 0) return 0;
    
    Move last_move = move_history[move_count - 1];
    char piece = board[src_row][src_col];
    
    // Check if last move was a two-square pawn advance
    if (tolower(last_move.piece) != 'p' || 
        abs(last_move.from_row - last_move.to_row) != 2) {
        return 0;
    }
    
    // Check if capturing pawn is in correct position
    if (is_white_piece(piece)) {
        if (src_row != 3 || dest_row != 2) return 0;
    } else {
        if (src_row != 4 || dest_row != 5) return 0;
    }
    
    // Check if capturing the correct pawn
    return (dest_col == last_move.to_col && abs(src_col - dest_col) == 1);
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

// void main(void) {
//     uart_init();
//     int num_games = 10;  // Set number of games to play
//     int white_wins = 0, black_wins = 0, draws = 0;
//     int total_moves = 0;  // Track total moves across all games
    
//     // Set search parameters
//     const int SEARCH_DEPTH = 8;  // THIS IS ONLY USED IN find_best_move line 4 ish !!!! ADJUST IT THERE 
//     g_max_positions = 20000;      // Adjust this for speed vs strength
    
//     uart_putstring("Starting AI Self-Play Chess Tournament\n");
//     uart_putstring("Search depth: ");
//     printf("%d", SEARCH_DEPTH);
//     uart_putstring(", Max positions: ");
//     printf("%d\n", g_max_positions);
//     uart_putstring("Number of games: ");
//     printf("%d\n\n", num_games);
    
//     for (int game = 1; game <= num_games; game++) {
//         reset_game();
//         int moves_this_game = 0;
//         uart_putstring("Game ");
//         printf("%d", game);
//         uart_putstring(" starting...\n");
//         display_board(board);
        
//         while (1) {  // Single game loop
//             // Reset position counter before each move
//             g_positions_examined = 0;
            
//             // White's turn
//             int is_checkmate;
//             if (is_game_over(board, &is_checkmate, 1)) {
//                 if (is_checkmate) {
//                     uart_putstring("\nCheckmate! Black wins!\n");
//                     black_wins++;
//                 } else {
//                     uart_putstring("\nStalemate! Game is a draw!\n");
//                     draws++;
//                 }
//                 break;
//             }

//             int white_move[4];
//             find_best_move(board, white_move, 1);  // This function should use SEARCH_DEPTH
            
//             // Check if valid move was found
//             if (white_move[0] == -1) {
//                 uart_putstring("\nNo legal moves for White! Draw!\n");
//                 draws++;
//                 break;
//             }
            
//             make_move(board, white_move[0], white_move[1], white_move[2], white_move[3], 1);
//             moves_this_game++;
//             uart_putstring("White played: ");
//             display_move(white_move[0], white_move[1], white_move[2], white_move[3]);
//             uart_putstring("\n");
//             display_board(board);

//             // After making move
//             if (game_state == DRAW_BY_REPETITION) {
//                 uart_putstring("\nDraw by threefold repetition!\n");
//                 draws++;
//                 break;
//             }
//             if (game_state == DRAW_BY_FIFTY_MOVE) {
//                 uart_putstring("\nDraw by fifty-move rule!\n");
//                 draws++;
//                 break;
//             }

//             // Reset position counter before black's move
//             g_positions_examined = 0;
            
//             // Black's turn
//             if (is_game_over(board, &is_checkmate, 0)) {
//                 if (is_checkmate) {
//                     uart_putstring("\nCheckmate! White wins!\n");
//                     white_wins++;
//                 } else {
//                     uart_putstring("\nStalemate! Game is a draw!\n");
//                     draws++;
//                 }
//                 break;
//             }

//             int black_move[4];
//             find_best_move(board, black_move, 0);  // This function should use SEARCH_DEPTH
            
//             // Check if valid move was found
//             if (black_move[0] == -1) {
//                 uart_putstring("\nNo legal moves for Black! Draw!\n");
//                 draws++;
//                 break;
//             }
            
//             make_move(board, black_move[0], black_move[1], black_move[2], black_move[3], 1);
//             moves_this_game++;
//             uart_putstring("Black played: ");
//             display_move(black_move[0], black_move[1], black_move[2], black_move[3]);
//             uart_putstring("\n");
//             display_board(board);
            
//             // Optional: Add a small delay between moves
//             // timer_delay_ms(500);
//         }

//         total_moves += moves_this_game;
//         uart_putstring("\nGame ");
//         printf("%d", game);
//         uart_putstring(" complete in ");
//         printf("%d", moves_this_game);
//         uart_putstring(" moves!\n");
//         uart_putstring("Current score - White: ");
//         printf("%d", white_wins);
//         uart_putstring(" Black: ");
//         printf("%d", black_wins);
//         uart_putstring(" Draws: ");
//         printf("%d", draws);
//         uart_putstring("\n\n");
//     }
    
//     // Final statistics
//     uart_putstring("\nTournament Complete!\n");
//     uart_putstring("Final Score:\n");
//     uart_putstring("White wins: ");
//     printf("%d", white_wins);
//     uart_putstring(" (");
//     printf("%d", (white_wins * 100) / num_games);
//     uart_putstring("%)\n");
//     uart_putstring("Black wins: ");
//     printf("%d", black_wins);
//     uart_putstring(" (");
//     printf("%d", (black_wins * 100) / num_games);
//     uart_putstring("%)\n");
//     uart_putstring("Draws: ");
//     printf("%d", draws);
//     uart_putstring(" (");
//     printf("%d", (draws * 100) / num_games);
//     uart_putstring("%)\n");
//     uart_putstring("Average moves per game: ");
//     printf("%d", total_moves / num_games);
//     uart_putstring("\n");
// }

void itoa(int num, char *str) {
    int i = 0;
    int is_negative = 0;

    // Handle 0 explicitly, otherwise empty string is printed for 0
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    // Handle negative numbers
    if (num < 0) {
        is_negative = 1;
        num = -num;
    }

    // Process individual digits
    while (num != 0) {
        int rem = num % 10;
        str[i++] = rem + '0';
        num = num / 10;
    }

    // If number is negative, append '-'
    if (is_negative) {
        str[i++] = '-';
    }

    str[i] = '\0'; // Append string terminator

    // Reverse the string
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

void uart_getstring(char *buffer, int length) {
    int i = 0;
    char c;
    while (i < length - 1) {
        c = uart_getchar();
        
        // Handle backspace
        if (c == '\b' || c == 127) {  // 127 is DEL
            if (i > 0) {
                uart_putstring("\b \b");  // Move back, clear char, move back again
                i--;
            }
            continue;
        }
        
        // Handle enter/return
        if (c == '\n' || c == '\r') {
            uart_putchar('\n');  // Echo newline
            break;
        }
        
        // Handle regular characters
        if (i < length - 1) {
            uart_putchar(c);  // Echo the character
            buffer[i++] = c;
        }
    }
    buffer[i] = '\0';  // Null-terminate the string
}

// void main(void) {     // USER IS WHITE HERE SWITCHED TO BLACK BELOW
//     uart_init();
//     int white_wins = 0, black_wins = 0, draws = 0;
//     char buffer[10];  // Buffer to hold converted numbers
    
//     reset_game();
//     display_board(board);
    
//     while (1) {  // Single game loop
//         char input[5];
//         int src_row, src_col, dest_row, dest_col;
        
//         // Reset position counter before white's move
//         g_positions_examined = 0;
        
//         // White's turn (User input)
//         uart_putstring("White's move (e.g., e2e4): ");
//         uart_getstring(input, sizeof(input));
//         parse_move(input, &src_row, &src_col, &dest_row, &dest_col);
        
//         if (!is_valid_move(board, src_row, src_col, dest_row, dest_col)) {
//             uart_putstring("Invalid move. Try again.\n");
//             continue;
//         }
        
//         make_move(board, src_row, src_col, dest_row, dest_col, 1);
//         display_board(board);
        
//         // Check for draws after white's move
//         if (game_state == DRAW_BY_REPETITION) {
//             uart_putstring("\nDraw by threefold repetition!\n");
//             draws++;
//             break;
//         }
//         if (game_state == DRAW_BY_FIFTY_MOVE) {
//             uart_putstring("\nDraw by fifty-move rule!\n");
//             draws++;
//             break;
//         }
        
//         // Check for game over conditions
//         int is_checkmate;
//         if (is_game_over(board, &is_checkmate, 0)) {
//             if (is_checkmate) {
//                 uart_putstring("Checkmate! White wins!\n");
//                 white_wins++;
//             } else {
//                 uart_putstring("Stalemate! Game is a draw!\n");
//                 draws++;
//             }
//             break;
//         }
        
//         // Reset position counter before black's move
//         g_positions_examined = 0;
        
//         // Black's turn (AI)
//         int black_move[4];
//         find_best_move(board, black_move, 0);  // AI generates move for Black
        
//         if (black_move[0] == -1) {
//             uart_putstring("\nNo legal moves for Black! Draw!\n");
//             draws++;
//             break;
//         }
        
//         make_move(board, black_move[0], black_move[1], black_move[2], black_move[3], 1);
//         uart_putstring("Black played: ");
//         display_move(black_move[0], black_move[1], black_move[2], black_move[3]);
//         uart_putstring("\n");
//         display_board(board);
        
//         // Check for draws after black's move
//         if (game_state == DRAW_BY_REPETITION) {
//             uart_putstring("\nDraw by threefold repetition!\n");
//             draws++;
//             break;
//         }
//         if (game_state == DRAW_BY_FIFTY_MOVE) {
//             uart_putstring("\nDraw by fifty-move rule!\n");
//             draws++;
//             break;
//         }
        
//         // Check for game over conditions
//         if (is_game_over(board, &is_checkmate, 1)) {
//             if (is_checkmate) {
//                 uart_putstring("Checkmate! Black wins!\n");
//                 black_wins++;
//             } else {
//                 uart_putstring("Stalemate! Game is a draw!\n");
//                 draws++;
//             }
//             break;
//         }
//     }
    
//     // Print final statistics
//     uart_putstring("Final Score:\n");
//     uart_putstring("White wins: ");
//     itoa(white_wins, buffer);
//     uart_putstring(buffer);
//     uart_putstring("\nBlack wins: ");
//     itoa(black_wins, buffer);
//     uart_putstring(buffer);
//     uart_putstring("\nDraws: ");
//     itoa(draws, buffer);
//     uart_putstring(buffer);
//     uart_putstring("\n");
// }

void main(void) {
    uart_init();
    int white_wins = 0, black_wins = 0, draws = 0;
    char buffer[10];  // Buffer to hold converted numbers
    
    reset_game();
    display_board(board);
    
    while (1) {  // Single game loop
        char input[5];
        int src_row, src_col, dest_row, dest_col;
        
        // Reset position counter before white's move
        g_positions_examined = 0;
        
        // White's turn (AI)
        int white_move[4];
        find_best_move(board, white_move, 1);  // AI generates move for White
        
        if (white_move[0] == -1) {
            uart_putstring("\nNo legal moves for White! Draw!\n");
            draws++;
            break;
        }
        
        make_move(board, white_move[0], white_move[1], white_move[2], white_move[3], 1);
        uart_putstring("White played: ");
        display_move(white_move[0], white_move[1], white_move[2], white_move[3]);
        uart_putstring("\n");
        display_board(board);
        
        // Check for draws after white's move
        if (game_state == DRAW_BY_REPETITION) {
            uart_putstring("\nDraw by threefold repetition!\n");
            draws++;
            break;
        }
        if (game_state == DRAW_BY_FIFTY_MOVE) {
            uart_putstring("\nDraw by fifty-move rule!\n");
            draws++;
            break;
        }
        
        // Check for game over conditions
        int is_checkmate;
        if (is_game_over(board, &is_checkmate, 0)) {
            if (is_checkmate) {
                uart_putstring("Checkmate! White wins!\n");
                white_wins++;
            } else {
                uart_putstring("Stalemate! Game is a draw!\n");
                draws++;
            }
            break;
        }
        
        // Reset position counter before black's move
        g_positions_examined = 0;
        
        // Black's turn (User input)
        while (1) {  // Loop until a valid move is made
            uart_putstring("Black's move (e.g., e7e5): ");
            uart_getstring(input, sizeof(input));
            parse_move(input, &src_row, &src_col, &dest_row, &dest_col);
            
            if (is_valid_move(board, src_row, src_col, dest_row, dest_col)) {
                break;  // Exit loop if move is valid
            } else {
                uart_putstring("Invalid move. Try again.\n");
            }
        }
        
        make_move(board, src_row, src_col, dest_row, dest_col, 1);
        display_board(board);
        
        // Check for draws after black's move
        if (game_state == DRAW_BY_REPETITION) {
            uart_putstring("\nDraw by threefold repetition!\n");
            draws++;
            break;
        }
        if (game_state == DRAW_BY_FIFTY_MOVE) {
            uart_putstring("\nDraw by fifty-move rule!\n");
            draws++;
            break;
        }
        
        // Check for game over conditions
        if (is_game_over(board, &is_checkmate, 1)) {
            if (is_checkmate) {
                uart_putstring("Checkmate! Black wins!\n");
                black_wins++;
            } else {
                uart_putstring("Stalemate! Game is a draw!\n");
                draws++;
            }
            break;
        }
    }
    
    // Print final statistics
    uart_putstring("Final Score:\n");
    uart_putstring("White wins: ");
    itoa(white_wins, buffer);
    uart_putstring(buffer);
    uart_putstring("\nBlack wins: ");
    itoa(black_wins, buffer);
    uart_putstring(buffer);
    uart_putstring("\nDraws: ");
    itoa(draws, buffer);
    uart_putstring(buffer);
    uart_putstring("\n");
}