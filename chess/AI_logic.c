// #include "move_validation.h"
// #include "game_state.h"
// #include "game_logic.h"
// #include "AI_logic.h"
// #include "move_generation.h"
// #include "board.h"
// #include "utils.h"
// #include "strings.h"
// #include "uart.h"

// int score_move(char board[8][8], int src_row, int src_col, 
//                int dest_row, int dest_col, int move_count) {
//     int score = 0;
//     char piece = board[src_row][src_col];
//     char captured = board[dest_row][dest_col];
    
//     // Early game focus (first 15 moves)
//     if (move_count < 15) {
//         // KNIGHTS FIRST - Highest priority
//         if (tolower(piece) == 'n') {
//             int home_rank = is_white_piece(piece) ? 7 : 0;
//             if (src_row == home_rank) {  // Knight hasn't moved yet
//                 score += 8000;  // Huge bonus for developing knights
//                 // Extra bonus for f3/c3 or f6/c6
//                 if ((is_white_piece(piece) && dest_row == 5 && (dest_col == 2 || dest_col == 5)) ||  // c3/f3
//                     (!is_white_piece(piece) && dest_row == 2 && (dest_col == 2 || dest_col == 5))) { // c6/f6
//                     score += 4000;
//                 }
//             }
//         }

//         // CENTER PAWNS SECOND
//         if (tolower(piece) == 'p') {
//             if ((src_col == 3 || src_col == 4) &&  // d or e pawn
//                 ((is_white_piece(piece) && src_row == 6 && dest_row == 4) ||  // d4/e4
//                  (!is_white_piece(piece) && src_row == 1 && dest_row == 3))) {  // d5/e5
//                 score += 6000;  // Strong bonus for initial center pawn moves
//             }
//             // Discourage wing pawn moves
//             if (src_col < 2 || src_col > 5) {  // a,b,g,h pawns
//                 score -= 5000;
//             }
//         }

//         // CASTLING THIRD
//         if (tolower(piece) == 'k') {
//             if (abs(dest_col - src_col) == 2) {
//                 score += 5000;  // Huge bonus for castling
//             } else {
//                 score -= 6000;  // Strong penalty for other king moves
//             }
//         }

//         // DISCOURAGE EARLY QUEEN MOVES
//         if (tolower(piece) == 'q') {
//             score -= 6000;
//         }
//     }
    
//     // Captures (lower priority in opening)
//     if (captured != '.') {
//         switch(tolower(captured)) {
//             case 'q': score += 900; break;
//             case 'r': score += 500; break;
//             case 'b': case 'n': score += 300; break;
//             case 'p': score += 100; break;
//         }
//     }
    
//     // Standard positional bonuses
//     char temp_board[8][8];
//     memcpy(temp_board, board, sizeof(temp_board));
//     make_move(temp_board, src_row, src_col, dest_row, dest_col, 0);
    
//     if (is_in_check(temp_board, !is_white_piece(piece))) {
//         score += 50;  // Small bonus for giving check
//     }
    
//     return score;
// }

// // Minimax with alpha-beta pruning
// int minimax(char board[8][8], int depth, int alpha, int beta, int maximizing_player, int current_move_count) {
//     g_positions_examined++;
//     if (g_positions_examined > g_max_positions) {
//         return evaluate_board(board, current_move_count);
//     }

//     int is_checkmate;
//     if (is_game_over(board, &is_checkmate, maximizing_player)) {
//         if (is_checkmate) {
//             return maximizing_player ? -20000 + depth : 20000 - depth;
//         }
//         return 0;
//     }
    
//     if (depth == 0) {
//         int eval = evaluate_board(board, current_move_count);
//         return eval;
//     }

//     int moves[200][4];
//     int move_count = 0;
    
//     typedef struct {
//         int move[4];
//         int score;
//     } ScoredMove;
//     ScoredMove scored_moves[200];

//     // Generate moves for the correct color
//     for (int i = 0; i < 8; i++) {
//         for (int j = 0; j < 8; j++) {
//             char piece = board[i][j];
//             if (maximizing_player ? is_white_piece(piece) : is_black_piece(piece)) {
//                 switch (tolower(piece)) {
//                     case 'p': generate_pawn_moves(board, i, j, moves, &move_count); break;
//                     case 'n': generate_knight_moves(board, i, j, moves, &move_count); break;
//                     case 'b': generate_bishop_moves(board, i, j, moves, &move_count); break;
//                     case 'r': generate_rook_moves(board, i, j, moves, &move_count); break;
//                     case 'q': generate_queen_moves(board, i, j, moves, &move_count); break;
//                     case 'k': generate_king_moves(board, i, j, moves, &move_count); break;
//                 }
//             }
//         }
//     }

//     // Score and store moves with early game development priority
//     for (int i = 0; i < move_count; i++) {
//         memcpy(scored_moves[i].move, moves[i], sizeof(int) * 4);
//         int base_score = score_move(board, 
//                                   moves[i][0], moves[i][1],
//                                   moves[i][2], moves[i][3],
//                                   current_move_count);
                                  
//         // Early game development priority
//         if (current_move_count < 10) {
//             char piece = board[moves[i][0]][moves[i][1]];
//             if (tolower(piece) == 'n' || tolower(piece) == 'b') {
//                 if ((maximizing_player && moves[i][0] == 7) || 
//                     (!maximizing_player && moves[i][0] == 0)) {
//                     base_score += 2000;
//                 }
//             }
//             if (tolower(piece) == 'p' && 
//                 (moves[i][1] < 2 || moves[i][1] > 5)) {
//                 base_score -= 1000;
//             }
//         }
        
//         scored_moves[i].score = base_score;
//     }

//     // Add randomization for equal moves in early game
//     if (current_move_count < 10) {  // Only in early game
//     unsigned int ticks = timer_get_ticks();
//     for (int i = 0; i < move_count; i++) {
//         // Add small random variation to all moves in opening
//         scored_moves[i].score += (ticks + i * 37) % 100;  // More variation
        
//         // Sometimes swap adjacent moves
//         if (i > 0 && ((ticks + i * 41) % 3 == 0)) {
//             ScoredMove temp = scored_moves[i];
//             scored_moves[i] = scored_moves[i-1];
//             scored_moves[i-1] = temp;
//         }
//     }
// }

//     // Sort moves by score
//     for (int i = 0; i < move_count - 1; i++) {
//         for (int j = 0; j < move_count - i - 1; j++) {
//             if (scored_moves[j].score < scored_moves[j + 1].score) {
//                 ScoredMove temp = scored_moves[j];
//                 scored_moves[j] = scored_moves[j + 1];
//                 scored_moves[j + 1] = temp;
//             }
//         }
//     }

//     if (maximizing_player) {
//         int max_eval = -20000;
//         for (int i = 0; i < move_count; i++) {
//             char temp_board[8][8];
//             memcpy(temp_board, board, sizeof(temp_board));
            
//             make_move(temp_board, scored_moves[i].move[0], scored_moves[i].move[1], 
//                                 scored_moves[i].move[2], scored_moves[i].move[3], 0);
//             int eval = minimax(temp_board, depth - 1, alpha, beta, 0, current_move_count + 1);
            
//             max_eval = eval > max_eval ? eval : max_eval;
//             alpha = alpha > eval ? alpha : eval;
//             if (beta <= alpha) break;
//         }
//         return max_eval;
//     } else {
//         int min_eval = 20000;
//         for (int i = 0; i < move_count; i++) {
//             char temp_board[8][8];
//             memcpy(temp_board, board, sizeof(temp_board));
            
//             make_move(temp_board, scored_moves[i].move[0], scored_moves[i].move[1], 
//                                 scored_moves[i].move[2], scored_moves[i].move[3], 0);
//             int eval = minimax(temp_board, depth - 1, alpha, beta, 1, current_move_count + 1);
//             min_eval = eval < min_eval ? eval : min_eval;
//             beta = beta < eval ? beta : eval;
//             if (beta <= alpha) break;
//         }
//         return min_eval;
//     }
// }

// // Update find_best_move to use minimax
// void find_best_move(char board[8][8], int best_move[4], int is_white) {
//     int moves[200][4];
//     int move_count = 0;
//     int best_score = is_white ? -10000 : 10000;
//     int depth = 4;

//     typedef struct {
//         int move[4];
//         int score;
//     } ScoredMove;
    
//     ScoredMove scored_moves[200];
    
//     // Generate moves for the correct color
//     for (int i = 0; i < 8; i++) {
//         for (int j = 0; j < 8; j++) {
//             char piece = board[i][j];
//             if ((is_white && is_white_piece(piece)) ||
//                 (!is_white && is_black_piece(piece))) {
//                 switch (tolower(piece)) {
//                     case 'p': generate_pawn_moves(board, i, j, moves, &move_count); break;
//                     case 'n': generate_knight_moves(board, i, j, moves, &move_count); break;
//                     case 'b': generate_bishop_moves(board, i, j, moves, &move_count); break;
//                     case 'r': generate_rook_moves(board, i, j, moves, &move_count); break;
//                     case 'q': generate_queen_moves(board, i, j, moves, &move_count); break;
//                     case 'k': generate_king_moves(board, i, j, moves, &move_count); break;
//                 }
//             }
//         }
//     }

//     // Score and store moves
//     for (int i = 0; i < move_count; i++) {
//         memcpy(scored_moves[i].move, moves[i], sizeof(int) * 4);
//         scored_moves[i].score = score_move(board, 
//                                          moves[i][0], moves[i][1],
//                                          moves[i][2], moves[i][3],
//                                          move_count);  // Just pass the existing move_count
//     }

//     // Sort moves (using insertion sort for better efficiency)
//     for (int i = 1; i < move_count; i++) {
//         ScoredMove key = scored_moves[i];
//         int j = i - 1;
//         while (j >= 0 && scored_moves[j].score < key.score) {
//             scored_moves[j + 1] = scored_moves[j];
//             j--;
//         }
//         scored_moves[j + 1] = key;
//     }

//     // Evaluate sorted moves
//     for (int i = 0; i < move_count; i++) {
//         char temp_board[8][8];
//         memcpy(temp_board, board, sizeof(temp_board));
        
//         make_move(temp_board, scored_moves[i].move[0], scored_moves[i].move[1], 
//                             scored_moves[i].move[2], scored_moves[i].move[3], 0);
        
//         int score = minimax(temp_board, depth - 1, -10000, 10000, !is_white, move_count);
        
//         if (is_white) {
//             if (score > best_score) {
//                 best_score = score;
//                 memcpy(best_move, scored_moves[i].move, sizeof(int) * 4);
//             }
//         } else {
//             if (score < best_score) {
//                 best_score = score;
//                 memcpy(best_move, scored_moves[i].move, sizeof(int) * 4);
//             }
//         }
//     }
    
//     // If no moves found, set invalid move
//     if (move_count == 0) {
//         best_move[0] = -1;
//         best_move[1] = -1;
//         best_move[2] = -1;
//         best_move[3] = -1;
//     }
// }

// int evaluate_position(char piece, int row, int col) {
//     int position_value = 0;
    
//     switch (tolower(piece)) {
//         case 'p': {
//             // Enhance pawn structure evaluation
//             if (col >= 2 && col <= 5) {
//                 position_value += 2;
//             }
//             if ((col == 3 || col == 4) && (row >= 2 && row <= 5)) {
//                 position_value += 3;
//             }
            
//             // Stronger advancement incentive in endgame
//             int endgame = (move_count > 30);
//             if (is_white_piece(piece)) {
//                 position_value += endgame ? (7 - row) : (7 - row) / 2;
//             } else {
//                 position_value += endgame ? row : row / 2;
//             }
            
//             // Penalize doubled pawns
//             int doubled = 0;
//             for (int r = 0; r < 8; r++) {
//                 if (r != row && tolower(board[r][col]) == 'p') {
//                     doubled = 1;
//                     break;
//                 }
//             }
//             if (doubled) position_value -= 5;
//             break;
//         }
//         case 'n': {
//             // More nuanced knight positioning
//             int center_dist = abs(3 - col) + abs(3 - row);
//             position_value += (6 - center_dist);  // Max bonus in center
            
//             // Knights are better with pawns nearby for protection
//             for (int dr = -1; dr <= 1; dr++) {
//                 for (int dc = -1; dc <= 1; dc++) {
//                     if (is_within_bounds(row + dr, col + dc)) {
//                         char nearby = board[row + dr][col + dc];
//                         if (tolower(nearby) == 'p' && 
//                             is_white_piece(nearby) == is_white_piece(piece)) {
//                             position_value += 1;
//                         }
//                     }
//                 }
//             }
//             break;
//         }
//         case 'b': {
//             // Enhanced bishop evaluation
//             if ((row + col) % 2 == 0) {
//                 position_value += 2;
//             }
            
//             // Count available diagonals
//             int mobility = 0;
//             for (int dr = -1; dr <= 1; dr += 2) {
//                 for (int dc = -1; dc <= 1; dc += 2) {
//                     int r = row + dr, c = col + dc;
//                     while (is_within_bounds(r, c) && board[r][c] == '.') {
//                         mobility++;
//                         r += dr;
//                         c += dc;
//                     }
//                 }
//             }
//             position_value += mobility / 2;
//             break;
//         }
//         case 'r': {
//             // Enhanced rook evaluation
//             int open_file = 1;
//             int semi_open = 1;
//             for (int r = 0; r < 8; r++) {
//                 if (r != row) {
//                     char piece_on_file = board[r][col];
//                     if (piece_on_file != '.') {
//                         open_file = 0;
//                         if (tolower(piece_on_file) == 'p' && 
//                             is_white_piece(piece_on_file) == is_white_piece(piece)) {
//                             semi_open = 0;
//                         }
//                     }
//                 }
//             }
//             if (open_file) position_value += 5;
//             else if (semi_open) position_value += 3;
            
//             // Bonus for 7th rank (2nd for black)
//             if ((is_white_piece(piece) && row == 1) || 
//                 (!is_white_piece(piece) && row == 6)) {
//                 position_value += 3;
//             }
//             break;
//         }
//         case 'k': {
//             // Different evaluation based on game phase
//             if (move_count < 20) {  // Opening/early middle game
//                 // Encourage castling position
//                 if (is_white_piece(piece)) {
//                     if (row == 7 && (col == 6 || col == 2)) position_value += 5;
//                 } else {
//                     if (row == 0 && (col == 6 || col == 2)) position_value += 5;
//                 }
//             } else {  // Late middle game/endgame
//                 // Encourage king activity
//                 int center_dist = abs(3 - col) + abs(3 - row);
//                 position_value += (7 - center_dist) / 2;
//             }
//             break;
//         }
//     }
    
//     return position_value;
// }

// int evaluate_board(char board[8][8], int current_move_count) {
//     int total_score = 0;
    
//     // Material values
//     const int PAWN_VALUE = 100;
//     const int KNIGHT_VALUE = 320;
//     const int BISHOP_VALUE = 330;
//     const int ROOK_VALUE = 500;
//     const int QUEEN_VALUE = 900;
//     const int KING_VALUE = 20000;
    
//     // Updated piece-square tables with stronger values
//     const int pawn_table[8][8] = {
//         { 0,  0,   0,   0,   0,   0,  0,  0},
//         {50, 50,  50,  50,  50,  50, 50, 50},
//         {10, 10, 100, 200, 200, 100, 10, 10},  // Much stronger center pawn bonus
//         { 5,  5,  50, 100, 100,  50,  5,  5},
//         { 0,  0,   0,  50,  50,   0,  0,  0},
//         { 5, -5, -10,   0,   0, -10, -5,  5},
//         {-5,-10,-500,-500,-500,-500,-10, -5},  // Heavy penalty for f2/f7
//         { 0,  0,   0,   0,   0,   0,  0,  0}
//     };
    
//     const int knight_table[8][8] = {
//         {-50,-40,-30,-30,-30,-30,-40,-50},
//         {-40,-20,  0,  5,  5,  0,-20,-40},
//         {-30, 25, 35, 40, 40, 35, 25,-30},
//         {-30, 35,400,450,450,400, 35,-30},  // Huge bonus for f3/c3
//         {-30, 25, 35, 40, 40, 35, 25,-30},
//         {-30,  0, 10, 15, 15, 10,  0,-30},
//         {-40,-20,  0,  5,  5,  0,-20,-40},
//         {-2000,-2000,-2000,-2000,-2000,-2000,-2000,-2000}  // Extreme back rank penalty
//     };
    
//     const int bishop_table[8][8] = {
//         {-20,-10,-10,-10,-10,-10,-10,-20},
//         {-10,  0, 20, 20, 20, 20,  0,-10},
//         {-10, 20, 25, 30, 30, 25, 20,-10},
//         {-10, 25, 25,100,100, 25, 25,-10},  // Better center control
//         {-10, 20, 30, 30, 30, 30, 20,-10},
//         {-10, 30, 30, 30, 30, 30, 30,-10},
//         {-10,  5,  0,  0,  0,  0,  5,-10},
//         {-2000,-2000,-2000,-2000,-2000,-2000,-2000,-2000}  // Extreme back rank penalty
//     };
    
//     const int rook_table[8][8] = {
//         { 0,  0,  0,  0,  0,  0,  0,  0},
//         { 5, 10, 10, 10, 10, 10, 10,  5},
//         {-5,  0,  0,  0,  0,  0,  0, -5},
//         {-5,  0,  0,  0,  0,  0,  0, -5},
//         {-5,  0,  0,  0,  0,  0,  0, -5},
//         {-5,  0,  0,  0,  0,  0,  0, -5},
//         {-5,  0,  0,  0,  0,  0,  0, -5},
//         { 0,  0,  0,  5,  5,  0,  0,  0}
//     };
    
//     const int queen_table[8][8] = {
//         {-20,-10,-10, -5, -5,-10,-10,-20},
//         {-10,-20,-20,-20,-20,-20,-20,-10},  // Discourage early queen moves
//         {-10,-20,  0,  0,  0,  0,-20,-10},  // Discourage early queen moves
//         { -5,  0,  5,  5,  5,  5,  0, -5},
//         {  0,  0,  5,  5,  5,  5,  0, -5},
//         {-10,  5,  5,  5,  5,  5,  0,-10},
//         {-10,  0,  5,  0,  0,  0,  0,-10},
//         {-20,-10,-10, -5, -5,-10,-10,-20}
//     };
    
//     const int king_table_middlegame[8][8] = {
//         {-30,-40,-40,-50,-50,-40,-40,-30},
//         {-30,-40,-40,-50,-50,-40,-40,-30},
//         {-50,-60,-60,-70,-70,-60,-60,-50},  // Increased penalties
//         {-70,-80,-80,-90,-90,-80,-80,-70},  // Much worse in center
//         {-70,-80,-80,-90,-90,-80,-80,-70},
//         {-50,-60,-60,-70,-70,-60,-60,-50},
//         {-10,-20,-20,-20,-20,-20,-20,-10},
//         { 20, 30, 10,  0,  0, 10, 30, 20}   // Back rank is safe
//     };
    
//     // Track development and center control
//     int white_developed_pieces = 0;
//     int black_developed_pieces = 0;
//     int white_center_pawns = 0;
//     int black_center_pawns = 0;
//     int white_queen_moved = 0;
//     int black_queen_moved = 0;
    
//     for (int i = 0; i < 8; i++) {
//         for (int j = 0; j < 8; j++) {
//             char piece = board[i][j];
//             if (piece == '.') continue;
            
//             int piece_value = 0;
//             int position_value = 0;
//             int row = is_white_piece(piece) ? i : 7-i;
            
//             switch(tolower(piece)) {
//                 case 'p':
//                     piece_value = PAWN_VALUE;
//                     position_value = pawn_table[row][j];
                    
//                     if (current_move_count < 20) {
//                         if (j == 3 || j == 4) {  // d or e pawn
//                             if (is_white_piece(piece)) {
//                                 white_center_pawns++;
//                                 position_value += 500;  // Much stronger center bonus
//                             } else {
//                                 black_center_pawns++;
//                                 position_value += 500;
//                             }
//                         }
//                         // Stronger penalty for wing pawns
//                         if ((j <= 1 || j >= 6) && 
//                             ((is_white_piece(piece) && i != 6) || 
//                              (!is_white_piece(piece) && i != 1))) {
//                             position_value -= 400;
//                         }
//                     }
//                     break;
                    
//                 case 'n':
//                     piece_value = KNIGHT_VALUE;
//                     position_value = knight_table[row][j];
                    
//                     if (current_move_count < 20) {
//                         if (is_white_piece(piece)) {
//                             if (i != 7) {  // Knight has moved
//                                 white_developed_pieces++;
//                                 if ((j >= 2 && j <= 5) && (i >= 2 && i <= 5)) {
//                                     position_value += 300;  // Better center control
//                                 }
//                                 // Huge bonus for f3/c3
//                                 if ((i == 5 && (j == 2 || j == 5))) {
//                                     position_value += 1000;
//                                 }
//                             } else {
//                                 position_value -= 400;  // Stronger back rank penalty
//                                 if (j == 6) position_value -= 1000;  // Blocking castle
//                             }
//                         } else {
//                             if (i != 0) {  // Knight has moved from back rank
//                                 black_developed_pieces++;
//                                 if ((j >= 2 && j <= 5) && (i >= 2 && i <= 5)) {
//                                     position_value += 30;
//                                 }
//                                 // HUGE bonus for moving kingside knight
//                                 if (j == 6) {
//                                     position_value += 200;
//                                 }
//                             } else {
//                                 position_value -= 40;
//                                 // Extra penalty for blocking castle
//                                 if (j == 6) position_value -= 200;
//                             }
//                         }
//                     }
//                     break;

//                 case 'b':
//                     piece_value = BISHOP_VALUE;
//                     position_value = bishop_table[row][j];
                    
//                     // Development tracking for bishops
//                     if (current_move_count < 20) {
//                         if (is_white_piece(piece)) {
//                             if (i != 7) {  // Bishop has moved from back rank
//                                 white_developed_pieces++;
//                                 // Extra bonus for controlling center squares
//                                 if ((j >= 2 && j <= 5) && (i >= 2 && i <= 5)) {
//                                     position_value += 30;
//                                 }
//                                 // HUGE bonus for moving kingside bishop
//                                 if (j == 5) {
//                                     position_value += 200;
//                                 }
//                             } else {
//                                 position_value -= 40;  // Penalty for staying on back rank
//                                 // Extra penalty for blocking castle
//                                 if (j == 5) position_value -= 200;
//                             }
//                         } else {
//                             if (i != 0) {  // Bishop has moved from back rank
//                                 black_developed_pieces++;
//                                 if ((j >= 2 && j <= 5) && (i >= 2 && i <= 5)) {
//                                     position_value += 30;
//                                 }
//                                 // HUGE bonus for moving kingside bishop
//                                 if (j == 5) {
//                                     position_value += 200;
//                                 }
//                             } else {
//                                 position_value -= 40;
//                                 // Extra penalty for blocking castle
//                                 if (j == 5) position_value -= 200;
//                             }
//                         }
//                     }
//                     break;
                    
//                 case 'r':
//                     piece_value = ROOK_VALUE;
//                     position_value = rook_table[row][j];
                    
//                     // Encourage rook activation
//                     if (current_move_count > 10) {  // After opening
//                         if (is_open_file(board, j)) {
//                             position_value += 30;  // Increased bonus
//                         }
//                         if ((is_white_piece(piece) && i == 1) || 
//                             (!is_white_piece(piece) && i == 6)) {
//                             position_value += 25;  // Bonus for 7th rank
//                         }
//                     }
//                     break;
                    
//                 case 'q':
//                     piece_value = QUEEN_VALUE;
//                     position_value = queen_table[row][j];
                    
//                     if (current_move_count < 15) {
//                         if (is_white_piece(piece)) {
//                             if (i != 7 || j != 3) {  // Queen moved
//                                 position_value -= 1000;  // Much stronger penalty
//                                 if (white_developed_pieces < 2) {
//                                     position_value -= 2000;  // Extra penalty if undeveloped
//                                 }
//                             }
//                         } else {
//                             if (i != 0 || j != 3) {
//                                 position_value += -80;
//                                 if (black_developed_pieces < 2) {
//                                     position_value += -80;
//                                 }
//                             }
//                         }
//                     }
//                     break;
                    
//                 case 'k':
//                     piece_value = KING_VALUE;

//                     if (!is_endgame(board)) {
//                         if (is_white_piece(piece)) {
//                             position_value = king_table_middlegame[i][j];
//                         } else {
//                             position_value = king_table_middlegame[7-i][j];
//                         }
                        
//                         int start_rank = is_white_piece(piece) ? 7 : 0;
//                         int start_file = 4;
                        
//                         // Check for successful castling first
//                         if (current_move_count < 30) {  // Early/mid game castling check
//                             if (is_white_piece(piece)) {
//                                 if (i == 7 && (j == 6 || j == 2)) {  // Castled
//                                     position_value += 10000;  // Huge castle bonus
//                                 } else if (i == 7 && j == 4 && current_move_count > 8) {
//                                     position_value -= 2000;  // Stronger penalty for not castling
//                                 }
//                             } else {
//                                 if (i == 0 && (j == 6 || j == 2)) {  // Kingside or Queenside castle
//                                     position_value += 5000;  // HUGE bonus for castling
//                                     // printf("\n!!! BLACK CASTLED !!!\n");
//                                 } else if (i == 0 && j == 4 && current_move_count > 8) {
//                                     position_value -= 1000;  // Penalty for not castling when possible
//                                 }
//                             }
//                         }

//                         // Your existing penalties for unsafe king moves
//                         if (i != start_rank || j != start_file) {
//                             // Only print for move counts 0-10, and only once per count
//                             static int last_printed_move = -1;
//                             if (current_move_count <= 10 && current_move_count != last_printed_move) {
//                                 last_printed_move = current_move_count;
//                             }
                            
//                             position_value -= 5000;  // Base penalty
//                             if (j >= 2 && j <= 5) {
//                                 position_value -= 2000;
//                             }
//                             if (i >= 2 && i <= 5) {
//                                 position_value -= 2000;
//                             }
//                             if (current_move_count < 20) {
//                                 position_value -= 5000;
//                             }
//                         }
//                     }
//                     break;
//             }
            
//             if (is_white_piece(piece)) {
//                 total_score += piece_value + position_value;
//             } else {
//                 total_score -= piece_value + position_value;
//             }
//         }
//     }
    
//     // Increase development bonuses in opening
//     if (current_move_count < 10) {
//         total_score += (white_developed_pieces - black_developed_pieces) * 40;  // Increased from 30
//         total_score += (white_center_pawns - black_center_pawns) * 50;  // Increased from 40
        
//         if (white_queen_moved) total_score -= 60;  // Increased from 40
//         if (black_queen_moved) total_score += 60;
//     }
    
//     // Add king safety evaluation for both sides
//     total_score += evaluate_king_safety(board, 1);  // White
//     total_score += evaluate_king_safety(board, 0);  // Black
    
//     // Opening principles (first 10 moves)
//     int opening_bonus = 0;
//     if (current_move_count < 20) {  // Both players' first 10 moves
//         for (int i = 0; i < 8; i++) {
//             for (int j = 0; j < 8; j++) {
//                 char piece = board[i][j];
//                 if (piece == '.') continue;
                
//                 // Center control bonus for pawns and knights
//                 if ((j == 3 || j == 4) && (i == 3 || i == 4)) {
//                     if (tolower(piece) == 'p' || tolower(piece) == 'n') {
//                         opening_bonus += is_white_piece(piece) ? 30 : -30;
//                     }
//                 }
                
//                 // Development bonus for minor pieces
//                 if (tolower(piece) == 'n' || tolower(piece) == 'b') {
//                     int start_rank = is_white_piece(piece) ? 7 : 0;
//                     if (i != start_rank) {
//                         opening_bonus += is_white_piece(piece) ? 20 : -20;
//                     }
//                 }
                
//                 // Penalize early queen moves
//                 if (tolower(piece) == 'q') {
//                     int start_rank = is_white_piece(piece) ? 7 : 0;
//                     if (i != start_rank) {
//                         opening_bonus += is_white_piece(piece) ? -40 : 40;
//                     }
//                 }
                
//                 // Castle bonus
//                 if (tolower(piece) == 'k') {
//                     if (is_white_piece(piece) && i == 7 && (j == 1 || j == 6)) {
//                         opening_bonus += 50;
//                     }
//                     if (!is_white_piece(piece) && i == 0 && (j == 1 || j == 6)) {
//                         opening_bonus -= 50;
//                     }
//                 }
//             }
//         }
//         total_score += opening_bonus;
//     }
    
//     // Add specific endgame scoring
//     if (is_endgame(board)) {
//         // Encourage pushing enemy king to the edge
//         int enemy_king_row, enemy_king_col;
//         find_king(board, &enemy_king_row, &enemy_king_col, !is_white_piece(board[0][0]));
        
//         // Distance from center penalty (force king to edge)
//         int center_dist = abs(3.5 - enemy_king_row) + abs(3.5 - enemy_king_col);
//         total_score += is_white_piece(board[0][0]) ? (center_dist * 10) : -(center_dist * 10);
        
//         // Distance between kings bonus (keep kings apart)
//         int friendly_king_row, friendly_king_col;
//         find_king(board, &friendly_king_row, &friendly_king_col, is_white_piece(board[0][0]));
//         int king_distance = abs(friendly_king_row - enemy_king_row) + 
//                            abs(friendly_king_col - enemy_king_col);
//         total_score += is_white_piece(board[0][0]) ? (king_distance * 5) : -(king_distance * 5);
        
//         // Heavy penalty for stalemate positions
//         if (!has_legal_moves(board, !is_white_piece(board[0][0])) && !is_in_check(board, !is_white_piece(board[0][0]))) {
//             total_score += is_white_piece(board[0][0]) ? -1000 : 1000;  // Heavily penalize stalemate
//         }

//         // Add heavy penalty for positions that could lead to stalemate
//         if (is_near_stalemate(board, is_white_piece(board[0][0]))) {
//             total_score += is_white_piece(board[0][0]) ? -800 : 800;  // Slightly less than actual stalemate
//         }
//     }
    
//     return is_white_piece(board[0][0]) ? total_score : -total_score;
// }

// int evaluate_king_safety(char board[8][8], int is_white) {
//     int king_row, king_col;
//     if (!find_king(board, &king_row, &king_col, is_white)) {
//         return 0;  // Safety check in case king not found
//     }
    
//     int safety_score = 0;
    
//     // Early/middlegame king safety (first 20 moves)
//     if (move_count < 20) {
//         // Severely penalize early king movement from back rank
//         int start_rank = is_white ? 7 : 0;
//         if (king_row != start_rank) {
//             safety_score -= 100;  // Increased penalty for early king movement
//         }
        
//         // Pawn shield - more important in early/middlegame
//         int pawn_rank = is_white ? king_row - 1 : king_row + 1;
//         if (pawn_rank >= 0 && pawn_rank < 8) {
//             for (int col = max(0, king_col - 1); col <= min(7, king_col + 1); col++) {
//                 if (board[pawn_rank][col] == (is_white ? 'P' : 'p')) {
//                     safety_score += 30;  // Increased bonus for pawn shield
//                 }
//             }
//         }
//     } else {
//         // Late game - less severe penalties
//         // Pawn shield still matters but less important
//         int pawn_rank = is_white ? king_row - 1 : king_row + 1;
//         if (pawn_rank >= 0 && pawn_rank < 8) {
//             for (int col = max(0, king_col - 1); col <= min(7, king_col + 1); col++) {
//                 if (board[pawn_rank][col] == (is_white ? 'P' : 'p')) {
//                     safety_score += 10;
//                 }
//             }
//         }
        
//         // Still penalize open files but less severely
//         for (int col = max(0, king_col - 1); col <= min(7, king_col + 1); col++) {
//             if (is_open_file(board, col)) {
//                 safety_score -= 20;
//             }
//         }
//     }
    
//     return is_white ? safety_score : -safety_score;
// }

// void generate_king_moves(char board[8][8], int row, int col, int moves[][4], int *move_count) {
//     char piece = board[row][col];
//     int start_rank = is_white_piece(piece) ? 7 : 0;
    
//     // Early game king move prevention
//     if (row == start_rank && col == 4) {
//         // Only generate moves if king is in check
//         if (!is_in_check(board, is_white_piece(piece))) {
//             return;  // Don't generate any moves if king is safe
//         }
//     }
    
//     int king_directions[8][2] = {
//         {-1, -1}, {-1, 0}, {-1, 1},
//         {0, -1},           {0, 1},
//         {1, -1},  {1, 0},  {1, 1}
//     };
    
//     for (int i = 0; i < 8; i++) {
//         int new_row = row + king_directions[i][0];
//         int new_col = col + king_directions[i][1];
        
//         if (is_within_bounds(new_row, new_col)) {
//             if (board[new_row][new_col] == '.' || 
//                 (is_black_piece(piece) ? is_white_piece(board[new_row][new_col]) 
//                                      : is_black_piece(board[new_row][new_col]))) {
//                 moves[*move_count][0] = row;
//                 moves[*move_count][1] = col;
//                 moves[*move_count][2] = new_row;
//                 moves[*move_count][3] = new_col;
//                 (*move_count)++;
//             }
//         }
//     }
// }

// void make_move(char board[8][8], int src_row, int src_col, int dest_row, int dest_col, int is_real_move) {
//     char piece = board[src_row][src_col];
//     char captured = board[dest_row][dest_col];
    
//     // Only update counters and history for real moves
//     if (is_real_move) {
//         // Reset counter on pawn move or capture
//         if (tolower(piece) == 'p' || captured != '.') {
//             moves_without_capture = 0;
//         } else {
//             moves_without_capture++;
//         }
        
//         // Store move in history
//         move_history[move_count].from_row = src_row;
//         move_history[move_count].from_col = src_col;
//         move_history[move_count].to_row = dest_row;
//         move_history[move_count].to_col = dest_col;
//         move_history[move_count].piece = piece;
//         move_history[move_count].captured = captured;
        
//         // Update king moved status for castling
//         if (tolower(piece) == 'k') {
//             if (is_white_piece(piece)) white_king_moved = 1;
//             else black_king_moved = 1;
//         }
        
//         // Update rook moved status
//         if (tolower(piece) == 'r') {
//             if (src_row == 7 && src_col == 0) white_rooks_moved[0] = 1;
//             if (src_row == 7 && src_col == 7) white_rooks_moved[1] = 1;
//             if (src_row == 0 && src_col == 0) black_rooks_moved[0] = 1;
//             if (src_row == 0 && src_col == 7) black_rooks_moved[1] = 1;
//         }
//     }
    
//     // Handle castling move (for both real and analysis moves)
//     if (tolower(piece) == 'k' && abs(dest_col - src_col) == 2) {
//         // Kingside castling
//         if (dest_col > src_col) {
//             board[src_row][5] = board[src_row][7];
//             board[src_row][7] = '.';
//         }
//         // Queenside castling
//         else {
//             board[src_row][3] = board[src_row][0];
//             board[src_row][0] = '.';
//         }
//     }
    
//     // Handle en passant capture
//     if (tolower(piece) == 'p' && src_col != dest_col && captured == '.') {
//         int captured_row = src_row;
//         board[captured_row][dest_col] = '.';
//     }
    
//     // Make the actual move
//     board[dest_row][dest_col] = piece;
//     board[src_row][src_col] = '.';
    
//     // Handle pawn promotion
//     if (tolower(piece) == 'p' && (dest_row == 0 || dest_row == 7)) {
//         char choice = is_white_piece(piece) ? 'Q' : 'q';
//         board[dest_row][dest_col] = choice;
//     }
    
//     // Only check game state for real moves
//     if (is_real_move) {
//         // Check for fifty move rule
//         if (moves_without_capture >= 100) {
//             game_state = DRAW_BY_FIFTY_MOVE;
//             return;
//         }
        
//         // Record position and check for repetition
//         int repetitions = record_position(board, !is_white_piece(piece), move_count);
//         if (repetitions >= 3) {
//             game_state = DRAW_BY_REPETITION;
//             uart_putstring("\nDraw by threefold repetition!\n");
//             return;
//         }
        
//         // Update move count after everything else
//         move_count++;
//     }
// }