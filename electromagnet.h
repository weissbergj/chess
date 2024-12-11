#ifndef ELECTROMAGNET_H
#define ELECTROMAGNET_H

void move_electromagnet(int row, int col);
void initialize_electromagnets(void);
int get_electromagnet_pin(int row, int col);
void convert_coordinates_to_chessboard(double x_cm, double y_cm, int *row, int *col);
void init_rotary_encoder(void);
int read_rotary_encoder(void);

#endif // ELECTROMAGNET_H