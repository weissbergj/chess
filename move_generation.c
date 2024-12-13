#include "move_generation.h"
#include "move_validation.h"  // For is_within_bounds, is_white_piece, is_black_piece
#include "board.h"
#include "game_logic.h"
#include "utils.h"
#include "uart.h"
#include "strings.h"
// #include "electromagnet.h"

void generate_knight_moves(char board[8][8], int row, int col, int moves[][4], int *move_count) {
    static const int knight_moves[8][2] = {
        {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
        {1, -2}, {1, 2}, {2, -1}, {2, 1}
    };
    
    char piece = board[row][col];
    
    for (int i = 0; i < 8; i++) {
        int new_row = row + knight_moves[i][0];
        int new_col = col + knight_moves[i][1];
        
        if (is_within_bounds(new_row, new_col)) {
            char target = board[new_row][new_col];
            // Check if the move is valid
            if (target == '.' || (is_black_piece(piece) ? is_white_piece(target) : is_black_piece(target))) {
                // Use the validation function to check if the move is valid
                if (is_valid_knight_move(board, row, col, new_row, new_col)) {
                    moves[*move_count][0] = row;
                    moves[*move_count][1] = col;
                    moves[*move_count][2] = new_row;
                    moves[*move_count][3] = new_col;
                    (*move_count)++;
                }
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
        // Simulate the move
        char temp_board[8][8];
        memcpy(temp_board, board, sizeof(temp_board));
        make_move(temp_board, row, col, row + direction, col, 0);
        
        // Check if the king is in check after the move
        if (!is_in_check(temp_board, is_white_piece(piece))) {
            moves[*move_count][0] = row;
            moves[*move_count][1] = col;
            moves[*move_count][2] = row + direction;
            moves[*move_count][3] = col;
            (*move_count)++;
        }
        
        // Initial two-square move
        if (row == start_row && board[row + 2 * direction][col] == '.') {
            // Simulate the two-square move
            memcpy(temp_board, board, sizeof(temp_board));
            make_move(temp_board, row, col, row + 2 * direction, col, 0);
            
            // Check if the king is in check after the move
            if (!is_in_check(temp_board, is_white_piece(piece))) {
                moves[*move_count][0] = row;
                moves[*move_count][1] = col;
                moves[*move_count][2] = row + 2 * direction;
                moves[*move_count][3] = col;
                (*move_count)++;
            }
        }
    }

    // Captures
    for (int dc = -1; dc <= 1; dc += 2) {
        if (is_within_bounds(row + direction, col + dc)) {
            char target = board[row + direction][col + dc];
            if (target != '.' && (is_black_piece(piece) ? is_white_piece(target) : is_black_piece(target))) {
                // Simulate the capture move
                char temp_board[8][8];
                memcpy(temp_board, board, sizeof(temp_board));
                make_move(temp_board, row, col, row + direction, col + dc, 0);
                
                // Check if the king is in check after the move
                if (!is_in_check(temp_board, is_white_piece(piece))) {
                    moves[*move_count][0] = row;
                    moves[*move_count][1] = col;
                    moves[*move_count][2] = row + direction;
                    moves[*move_count][3] = col + dc;
                    (*move_count)++;
                }
            }
        }
    }
}

void generate_sliding_moves(char board[8][8], int row, int col, const int directions[][2], 
                            int num_directions, int moves[][4], int *move_count) {
    char piece = board[row][col];
    
    for (int d = 0; d < num_directions; d++) {
        int dr = directions[d][0];
        int dc = directions[d][1];
        int new_row = row + dr;
        int new_col = col + dc;
        
        while (is_within_bounds(new_row, new_col)) {
            char target = board[new_row][new_col];
            // Simulate the move
            char temp_board[8][8];
            memcpy(temp_board, board, sizeof(temp_board));
            
            if (target == '.') {
                // Check if the move is valid
                make_move(temp_board, row, col, new_row, new_col, 0);
                if (!is_in_check(temp_board, is_white_piece(piece))) {
                    moves[*move_count][0] = row;
                    moves[*move_count][1] = col;
                    moves[*move_count][2] = new_row;
                    moves[*move_count][3] = new_col;
                    (*move_count)++;
                }
            } else {
                // Check for captures
                if ((is_black_piece(piece) && is_white_piece(target)) ||
                    (is_white_piece(piece) && is_black_piece(target))) {
                    // Check if the move is valid
                    make_move(temp_board, row, col, new_row, new_col, 0);
                    if (!is_in_check(temp_board, is_white_piece(piece))) {
                        moves[*move_count][0] = row;
                        moves[*move_count][1] = col;
                        moves[*move_count][2] = new_row;
                        moves[*move_count][3] = new_col;
                        (*move_count)++;
                    }
                }
                break; // Stop sliding after capturing
            }
            new_row += dr;
            new_col += dc;
        }
    }
}

// Generate all possible rook moves from a given position
void generate_rook_moves(char board[8][8], int row, int col, int moves[][4], int *move_count) {
    static const int rook_directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    generate_sliding_moves(board, row, col, rook_directions, 4, moves, move_count);
}

// Generate all possible bishop moves from a given position
void generate_bishop_moves(char board[8][8], int row, int col, int moves[][4], int *move_count) {
    static const int bishop_directions[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
    generate_sliding_moves(board, row, col, bishop_directions, 4, moves, move_count);
}

// Generate all possible queen moves from a given position
void generate_queen_moves(char board[8][8], int row, int col, int moves[][4], int *move_count) {
    generate_rook_moves(board, row, col, moves, move_count);
    generate_bishop_moves(board, row, col, moves, move_count);
}

// Generate all possible king moves from a given position
void generate_king_moves(char board[8][8], int row, int col, int moves[][4], int *move_count) {
    static const int king_directions[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        {0, -1},           {0, 1},
        {1, -1},  {1, 0},  {1, 1}
    };

    char piece = board[row][col];
    
    // Early game king move prevention: only generate moves if the king is in check
    if (row == (is_white_piece(piece) ? 7 : 0) && col == 4 && !is_in_check(board, is_white_piece(piece))) {
        return;
    }
    
    for (int i = 0; i < 8; i++) {
        int new_row = row + king_directions[i][0];
        int new_col = col + king_directions[i][1];
        
        if (is_within_bounds(new_row, new_col)) {
            char target = board[new_row][new_col];
            if (target == '.' || (is_black_piece(piece) ? is_white_piece(target) : is_black_piece(target))) {
                moves[*move_count][0] = row;
                moves[*move_count][1] = col;
                moves[*move_count][2] = new_row;
                moves[*move_count][3] = new_col;
                (*move_count)++;
            }
        }
    }
}

// Parse a move from input string to board coordinates
void parse_move(char *input, int *src_row, int *src_col, int *dest_row, int *dest_col) {
    *src_col = input[0] - 'a';       // Column 'a'-'h' -> 0-7
    *src_row = '8' - input[1];       // Row '8'-'1' -> 0-7
    *dest_col = input[2] - 'a';      // Column 'a'-'h' -> 0-7
    *dest_row = '8' - input[3];      // Row '8'-'1' -> 0-7
}

void make_move(char board[8][8], int src_row, int src_col, int dest_row, int dest_col, int is_real_move) {
    char piece = board[src_row][src_col];
    char captured = board[dest_row][dest_col];

    // Update move history and counters for real moves
    if (is_real_move) {
        moves_without_capture = (tolower(piece) == 'p' || captured != '.') ? 0 : moves_without_capture + 1;

        // Store move in history
        move_history[move_count] = (Move){src_row, src_col, dest_row, dest_col, piece, captured};

        // Update moved status for castling and rook moves
        if (tolower(piece) == 'k') {
            if (is_white_piece(piece)) white_king_moved = 1;
            else black_king_moved = 1;
        } else if (tolower(piece) == 'r') {
            if (src_row == 7) white_rooks_moved[src_col == 0 ? 0 : 1] = 1;
            if (src_row == 0) black_rooks_moved[src_col == 0 ? 0 : 1] = 1;
        }
    }

    // Handle castling
    if (tolower(piece) == 'k' && abs(dest_col - src_col) == 2) {
        int rook_src_col = (dest_col > src_col) ? 7 : 0;
        int rook_dest_col = (dest_col > src_col) ? 5 : 3;
        board[src_row][rook_dest_col] = board[src_row][rook_src_col];
        board[src_row][rook_src_col] = '.';
    }

    // Handle en passant
    if (tolower(piece) == 'p' && src_col != dest_col && captured == '.') {
        board[src_row][dest_col] = '.';
    }

    // Make the actual move
    board[dest_row][dest_col] = piece;
    board[src_row][src_col] = '.';

    // Handle pawn promotion
    if (tolower(piece) == 'p' && (dest_row == 0 || dest_row == 7)) {
        board[dest_row][dest_col] = is_white_piece(piece) ? 'Q' : 'q';
    }

    // Update game state for real moves
    if (is_real_move) {
        if (moves_without_capture >= 100) {
            game_state = DRAW_BY_FIFTY_MOVE;
            return;
        }

        if (record_position(board, !is_white_piece(piece), move_count) >= 3) {
            game_state = DRAW_BY_REPETITION;
            uart_putstring("\nDraw by threefold repetition!\n");
            return;
        }

        move_count++;
    }

    // Move the sliding electromagnet to the new position
    // move_electromagnet(dest_row, dest_col);
}