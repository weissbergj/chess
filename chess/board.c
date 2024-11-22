#include "move_validation.h"
#include "game_state.h"
#include "game_logic.h"
#include "move_generation.h"
#include "board.h"
#include "utils.h"
#include "strings.h"
#include "uart.h"

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

int count_pieces(char board[8][8]) {
    int count = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board[i][j] != '.') count++;
        }
    }
    return count;
}

int is_within_bounds(int row, int col) {
    return row >= 0 && row < 8 && col >= 0 && col < 8;
}