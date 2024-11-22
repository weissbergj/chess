#ifndef GAME_STATE_H
#define GAME_STATE_H

#define MAX_HISTORY 1000
#define MAX_POSITIONS 100

typedef struct {
    int from_row;
    int from_col;
    int to_row;
    int to_col;
    char piece;
    char captured;
} Move;

typedef struct {
    char board[8][8];
    int is_white_turn;
    int move_count;
} Position;

typedef enum {
    PLAYING,
    DRAW_BY_REPETITION,
    DRAW_BY_FIFTY_MOVE,
    CHECKMATE,
    STALEMATE
} GameState;

extern char board[8][8];
extern char initial_board[8][8];
extern Move move_history[MAX_HISTORY];
extern int move_count;
extern int white_king_moved;
extern int black_king_moved;
extern int white_rooks_moved[2];
extern int black_rooks_moved[2];
extern int moves_without_capture;
extern Position position_history[MAX_POSITIONS];
extern int position_count;
extern GameState game_state;
extern int g_positions_examined;
extern int g_max_positions;

void reset_game();
int record_position();
int is_repetition();
int is_insufficient_material();
int is_endgame();
int is_near_stalemate();
int is_open_file();
int is_white_piece(char piece);
int is_black_piece(char piece);
int can_capture(char src_piece, char dest_piece);

#endif // GAME_STATE_H