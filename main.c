/* File: main.c
 * -------------
 * Main program.
 */
#include "minimax.h"
#include "board.h"
#include "move_generation.h"
#include "move_validation.h"
#include "game_logic.h"
#include "utils.h"
#include "game_state.h"
#include "strings.h"
#include "uart.h"
#include "timer.h"

// Function to check game over conditions and handle outcomes
void check_game_over(int is_checkmate, int *white_wins, int *black_wins, int *draws, int player) {
    if (is_checkmate) {
        if (player == 0) {
            uart_putstring("Checkmate! White wins!\n");
            (*white_wins)++;
        } else {
            uart_putstring("Checkmate! Black wins!\n");
            (*black_wins)++;
        }
    } else {
        uart_putstring("Stalemate! Game is a draw!\n");
        (*draws)++;
    }
}

void main(void) {
    uart_init();
    int white_wins = 0, black_wins = 0, draws = 0;
    char buffer[10];  // Buffer to hold converted numbers
    
    reset_game();
    display_board(board);
    
    // Ask user if they want to play against the AI or let AI play itself
    uart_putstring("Would you like to play against the AI? (yes/no): ");
    char play_option[4];
    uart_getstring(play_option, sizeof(play_option));
    
    int user_playing = (play_option[0] == 'y' || play_option[0] == 'Y');

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
        
        // Check for game over conditions after white's move
        int is_checkmate;
        if (is_game_over(board, &is_checkmate, 0)) {
            check_game_over(is_checkmate, &white_wins, &black_wins, &draws, 0);
            break;
        }
        
        // Reset position counter before black's move
        g_positions_examined = 0;
        
        // Black's turn (User input or AI)
        if (user_playing) {
            while (1) {  // Loop until a valid move is made
                uart_putstring("Black's move (e.g., e7e5): ");
                uart_getstring(input, sizeof(input));
                parse_move(input, &src_row, &src_col, &dest_row, &dest_col);
                
                if (is_valid_move(board, src_row, src_col, dest_row, dest_col)) {
                    make_move(board, src_row, src_col, dest_row, dest_col, 0); // Execute the user's move
                    break;  // Exit loop if move is valid
                } else {
                    uart_putstring("Invalid move. Try again.\n");
                }
            }
        } else {
            // AI generates move for Black
            int black_move[4];
            find_best_move(board, black_move, 0);  // AI generates move for Black
            
            if (black_move[0] == -1) {
                uart_putstring("\nNo legal moves for Black! Draw!\n");
                draws++;
                break;
            }
            
            make_move(board, black_move[0], black_move[1], black_move[2], black_move[3], 0);
            uart_putstring("Black played: ");
            display_move(black_move[0], black_move[1], black_move[2], black_move[3]);
            uart_putstring("\n");
        }
        
        display_board(board);
        
        // Check for game over conditions after black's move
        if (is_game_over(board, &is_checkmate, 1)) {
            check_game_over(is_checkmate, &white_wins, &black_wins, &draws, 1);
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