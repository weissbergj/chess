// This file has two implementations of the same function. The first is commented out and allows for coordination and calculation of angles using inverse kinematics. These map to 
// the exact array used by 


// THE BELOW IS THE COMPLETE IMPLEMENTATION FOR MAPPING CHESS COORDINATES TO MOTOR MOVES

// #include "uart.h"
// #include "mymodule.h"
// #include "servo.h"
// #include "gpio.h"
// #include "ik.h"
// #include "printf.h"
// #include "electromagnet.h"
// #include "timer.h"
// #include "interrupts.h"
// #include "rotaryencoder.h"
// #include "math_float.h"

// // Define constants
// #define PULSES_PER_360_CENTER 20
// #define PULSES_PER_360_OUTER  20
// #define MAX_CENTER_ANGLE 360
// #define MAX_OUTER_ANGLE 180
// #define BOARD_SIZE 8
// #define SQUARE_SIZE_CM 5
// #define SCALE_FACTOR 100
// #define INITIAL_X_CM 1
// #define INITIAL_Y_CM 1

// // The center of the board in terms of its squares for a 41x41 board is at (20.5, 20.5).
// #define BOARD_CENTER ((float)BOARD_SIZE / 2.0f)

// // If you need to adjust final angles to match your robot's reference frame, define offsets here:
// #define THETA1_OFFSET 0.0f
// #define THETA2_OFFSET 169.8f

// // Function prototypes
// static int chess_to_scaled_cm(int row, int col, int* x_scaled, int* y_scaled);
// static void print_scaled_cm(int value_scaled);
// static void print_float_as_int_decimal(float value);
// static void move_electromagnet(int x_scaled, int y_scaled, servo_motor* servo_centre, servo_motor* servo_outer, 
//                                rotator* centre_encoder, rotator* outer_encoder,
//                                float *sum_theta1, float *sum_theta2);
// static void move_piece(int src_row, int src_col, int dest_row, int dest_col,
//                        servo_motor* servo_centre, servo_motor* servo_outer,
//                        rotator* centre_encoder, rotator* outer_encoder);
// int calculate_angles(float x, float y, float *theta1, float *theta2);

// // Function implementations

// static void print_scaled_cm(int value_scaled) {
//     int integer_part = value_scaled / SCALE_FACTOR;
//     int fractional_part = (value_scaled % SCALE_FACTOR) / 10; // One decimal place
//     printf("%d.%d cm", integer_part, fractional_part);
// }

// static void print_float_as_int_decimal(float value) {
//     int sign = (value < 0) ? -1 : 1;
//     value = value * sign;

//     int integer_part = (int)value;
//     int decimal_part = (int)((value - integer_part) * 10); // One decimal place

//     if (sign == -1) {
//         printf("-%d.%d", integer_part, decimal_part);
//     } else {
//         printf("%d.%d", integer_part, decimal_part);
//     }
// }

// // Map chessboard position to scaled coordinates, with (0,0) at the board center.
// // For BOARD_SIZE=41, center is at (20.5, 20.5).
// // x_cm = (col - 20.5 + 0.5)*SQUARE_SIZE_CM
// // y_cm = ((20.5 - row - 0.5)*SQUARE_SIZE_CM)
// // Note: Adjust signs for y depending on how you define "up" and "down".
// static int chess_to_scaled_cm(int row, int col, int* x_scaled, int* y_scaled) {
//     if (col < 0 || col >= BOARD_SIZE || row < 0 || row >= BOARD_SIZE) {
//         printf("Invalid chess position: (%d, %d)\n", row, col);
//         return -1;
//     }

//     float x_cm = ((float)col - BOARD_CENTER + 0.5f) * SQUARE_SIZE_CM;
//     float y_cm = ((BOARD_CENTER - (float)row - 0.5f) * SQUARE_SIZE_CM);

//     // Convert to scaled integers
//     *x_scaled = (int)(x_cm * SCALE_FACTOR);
//     *y_scaled = (int)(y_cm * SCALE_FACTOR);

//     return 0;
// }

// // Move by an angle difference using rotate()
// static void move_servo_by_angle(float angle, servo_motor* servo, rotator* encoder) {
//     // This will rotate by 'angle' degrees relative to current position.
//     rotate(angle, *servo, encoder);
// }

// // Move electromagnet to a given (x_scaled, y_scaled) by computing IK and rotating servos.
// // sum_theta1 and sum_theta2 store the cumulative angles moved so we can return to start by reversing them.
// static void move_electromagnet(int x_scaled, int y_scaled,
//                                servo_motor* servo_centre, servo_motor* servo_outer,
//                                rotator* centre_encoder, rotator* outer_encoder,
//                                float *sum_theta1, float *sum_theta2) {
//     float theta1 = 0.0f;
//     float theta2 = 0.0f;

//     // Print the target coordinates
//     printf("Input coordinates: x_cm = ");
//     print_scaled_cm(x_scaled);
//     printf(", y_cm = ");
//     print_scaled_cm(y_scaled);
//     printf("\n");

//     // Convert scaled to cm
//     float x_cm = x_scaled / (float)SCALE_FACTOR;
//     float y_cm = y_scaled / (float)SCALE_FACTOR;

//     // Calculate angles using IK
//     calculate_angles(x_cm, y_cm, &theta1, &theta2);

//     // Apply known offsets to align the robot's actual orientation
//     theta1 -= THETA1_OFFSET;
//     theta2 -= THETA2_OFFSET;

//     // Subtract the sum of previous moves to get relative movement
//     theta1 -= *sum_theta1;
//     theta2 -= *sum_theta2;

//     printf("Calculated angles (with offsets): theta1 = ");
//     print_float_as_int_decimal(theta1);
//     printf(", theta2 = ");
//     print_float_as_int_decimal(theta2);
//     printf("\n");

//     // Move the centre (theta1) first
//     printf("moving motor centre (theta1)\n");
//     move_servo_by_angle(theta1, servo_centre, centre_encoder);
//     timer_delay_us(100000);

//     // Then move the outer (theta2)
//     printf("moving motor outer (theta2)\n");
//     move_servo_by_angle(theta2, servo_outer, outer_encoder);
//     timer_delay_us(100000);

//     // Update cumulative angles
//     *sum_theta1 += theta1;
//     *sum_theta2 += theta2;
// }


// // static void move_pieces2(float src_row, float src_col, float dest_row, float dest_col,
// //                        servo_motor* servo_centre, servo_motor* servo_outer,
// //                        rotator* centre_encoder, rotator* outer_encoder) {
// //     float src_x, src_y, dest_x, dest_y;
// //     if (chess_to_scaled_cm(src_row, src_col, &src_x, &src_y) != 0) return;
// //     if (chess_to_scaled_cm(dest_row, dest_col, &dest_x, &dest_y) != 0) return;

// //     // We'll track how much we've moved so we can return to start
// //     float sum_theta1 = 0.0f;
// //     float sum_theta2 = 0.0f;

// //     // Move to source
// //     printf("Moving to source position: (%d, %d) (", (int)src_row, (int)src_col);
// //     print_scaled_cm((int)src_x);
// //     printf(", ");
// //     print_scaled_cm((int)src_y);
// //     printf(")\n");
// //     move_electromagnet(src_x, src_y, servo_centre, servo_outer, centre_encoder, outer_encoder, &sum_theta1, &sum_theta2);

// //     // Activate electromagnet
// //     printf("Activating electromagnet at source position.\n");
// //     set_electromagnet(1);

// //     // Moving magnets in between rows and cols by incrementing by 0.5
// //     float curr_x, curr_y;
// //     float curr_row = (src_row - dest_row <= 0) && (src_row != BOARD_SIZE - 1) ? src_row + 0.5 : src_row - 0.5;
// //     float curr_col = (src_col - dest_col <= 0) && (src_col != BOARD_SIZE - 1) ? src_col + 0.5: src_row - 0.5;
// //     if (chess_to_scaled_cm(curr_row, curr_col, &curr_x, &curr_y) != 0) return;
// //     move_electromagnet(curr_x, curr_y, servo_centre, servo_outer, centre_encoder, outer_encoder, &sum_theta1, &sum_theta2);
// //     float step_row = (src_row - dest_row <= 0) ? 0.5 : -0.5;
// //     float step_col =  (src_col - dest_col <= 0) ? 0.5 : -0.5;
   

// //     // Find correct row
// //     while (curr_row != dst_row + step_row) {     
// //         curr_row += step_row;
// //         if (chess_to_scaled_cm(curr_row, curr_col, &curr_x, &curr_y) != 0) return;
// //         move_electromagnet(curr_x, curr_y, servo_centre, servo_outer, centre_encoder, outer_encoder, &sum_theta1, &sum_theta2);
// //     }    

// //     while (curr_col != dst_col + step_col) {
// //         curr_col += step_col;
// //         if (chess_to_scaled_cm(curr_row, curr_col, &curr_x, &curr_y) != 0) return;
// //         move_electromagnet(curr_x, curr_y, servo_centre, servo_outer, centre_encoder, outer_encoder, &sum_theta1, &sum_theta2);
// //     }      

// //     // Here we are on the corner of the target destination square so we increment by half the step size to reach final position
// //     curr_row += step_row;
// //     if (chess_to_scaled_cm(curr_row, curr_col, &curr_x, &curr_y) != 0) return;
// //     move_electromagnet(curr_x, curr_y, servo_centre, servo_outer, centre_encoder, outer_encoder, &sum_theta1, &sum_theta2);

// //     curr_col += step_col;
// //     if (chess_to_scaled_cm(curr_row, curr_col, &curr_x, &curr_y) != 0) return;
// //     

    


// //     // Deactivate electromagnet
// //     printf("Deactivating electromagnet at destination position.\n");
// //     set_electromagnet(0);

// //     // Return to initial position by negating all moves
// //     // We moved sum_theta1 and sum_theta2 from start, so just rotate back by -sum_theta2 then -sum_theta1
// //     // We reverse the order to retrace our steps correctly:
// //     // Last servo moved was outer, then centre. So to go back:
// //     // move outer by -theta2, then centre by -theta1 in reverse order as needed.

// //     move_electromagnet(3.5, 3.5, servo_centre, servo_outer, centre_encoder, outer_encoder, &sum_theta1, &sum_theta2);
// //     printf("Returning to initial position.\n");
// //     // We made moves in order: first theta1 (centre), then theta2 (outer) at each step.
// //     // To undo: first undo outer moves: rotate(-sum_theta2), then undo centre moves: rotate(-sum_theta1).
// //     move_servo_by_angle(-sum_theta2, servo_outer, outer_encoder);
// //     move_servo_by_angle(-sum_theta1, servo_centre, centre_encoder);
// // }


// // Move a piece by going from initial to source, pick piece, source to destination, drop piece, and return home.
// static void move_piece(int src_row, int src_col, int dest_row, int dest_col,
//                        servo_motor* servo_centre, servo_motor* servo_outer,
//                        rotator* centre_encoder, rotator* outer_encoder) {
//     int src_x, src_y, dest_x, dest_y;
//     if (chess_to_scaled_cm(src_row, src_col, &src_x, &src_y) != 0) return;
//     if (chess_to_scaled_cm(dest_row, dest_col, &dest_x, &dest_y) != 0) return;

//     // We'll track how much we've moved so we can return to start
//     float sum_theta1 = 0.0f;
//     float sum_theta2 = 0.0f;
//     // NEW NEW 
//     // set_electromagnet(1);

//     // Move to source
//     printf("Moving to source position: (%d, %d) (", (int)src_row, (int)src_col);
//     print_scaled_cm((int)src_x);
//     printf(", ");
//     print_scaled_cm(src_y);
//     printf(")\n");
//     move_electromagnet(src_x, src_y, servo_centre, servo_outer, centre_encoder, outer_encoder, &sum_theta1, &sum_theta2);

//     timer_delay_us(100000);

//     // Activate electromagnet
//     printf("Activating electromagnet at source position.\n");
//     set_electromagnet(1);
//     timer_delay_us(100000);

//     // Move to destination
//     printf("Moving to destination position: (%d, %d) (", (int)dest_row, (int)dest_col);
//     print_scaled_cm(dest_x);
//     printf(", ");
//     print_scaled_cm(dest_y);
//     printf(")\n");
//     move_electromagnet(dest_x, dest_y, servo_centre, servo_outer, centre_encoder, outer_encoder, &sum_theta1, &sum_theta2);
//     timer_delay_us(100000);

//     // Deactivate electromagnet
//     printf("Deactivating electromagnet at destination position.\n");
//     set_electromagnet(0);
//     timer_delay_us(100000);

//     // Return to initial position by negating all moves
//     // We moved sum_theta1 and sum_theta2 from start, so just rotate back by -sum_theta2 then -sum_theta1
//     // We reverse the order to retrace our steps correctly:
//     // Last servo moved was outer, then centre. So to go back:
//     // move outer by -theta2, then centre by -theta1 in reverse order as needed.

//     printf("Returning to initial position.\n");
//     // We made moves in order: first theta1 (centre), then theta2 (outer) at each step.
//     // To undo: first undo outer moves: rotate(-sum_theta2), then undo centre moves: rotate(-sum_theta1).
//     move_servo_by_angle(-sum_theta2, servo_outer, outer_encoder);
//     timer_delay_us(100000);
//     move_servo_by_angle(-sum_theta1, servo_centre, centre_encoder);
//     timer_delay_us(100000);
// }

// int main(void) {
//     interrupts_init();
//     uart_init();

//     // electromagnet_init(GPIO_PB2); //DO I NEED THIS??

//     rotator* centre_encoder = rotary_init(GPIO_PB1, GPIO_PD14);
//     rotator* outer_encoder = rotary_init(GPIO_PC1, GPIO_PD10);

//     servo_motor servo_centre;
//     servo_motor servo_outer;

//     servo_centre.pin = GPIO_PB4;
//     servo_centre.type = CENTRE;
//     servo_centre.rotation = FULL_ROTATION_CENTRE;

//     servo_outer.pin = GPIO_PB3;
//     servo_outer.type = OUTER;
//     servo_outer.rotation = FULL_ROTATION_OUTER;

//     servo_init(servo_outer);
//     servo_init(servo_centre);

//     interrupts_global_enable();

//     // set_electromagnet(1);
//     // timer_delay_us(1000000000);

//     // Example move: Move piece from (1,1) to (3,3)
//     // This will:
//     // 1. Move from init to (1,1)
//     // 2. Pick piece
//     // 3. Move to (3,3)
//     // 4. Drop piece
//     // 5. Return back to initial position by reversing the ticks
//     move_piece(1, 1, 3, 3, &servo_centre, &servo_outer, centre_encoder, outer_encoder);

//     while(true) {}

//     say_hello("CS107e");
//     return 0;
// }


// THE BELOW IS CODE USED DIRECTLY IN THE DEMO THAT SHOWS A SIMPLE MOVE
#include "uart.h"
#include "mymodule.h"
#include "servo.h"
#include "gpio.h"
#include "ik.h"
#include "printf.h"
#include "electromagnet.h"
#include "timer.h"
#include "interrupts.h"
#include "rotaryencoder.h"
#include "math_float.h"

#define PULSES_PER_360_CENTER 20
#define PULSES_PER_360_OUTER  20
#define MAX_CENTER_ANGLE 360
#define MAX_OUTER_ANGLE 180
#define SCALE_FACTOR 100

static int chess_to_scaled_cm(int row, int col, int* x_scaled, int* y_scaled);
static void print_scaled_cm(int value_scaled);
static void print_float_as_int_decimal(float value);
static void move_electromagnet(float theta1, float theta2, servo_motor* servo_centre, servo_motor* servo_outer, 
                               rotator* centre_encoder, rotator* outer_encoder);
static void move_piece(float src_row, float src_col, float dest_row, float dest_col,
                       servo_motor* servo_centre, servo_motor* servo_outer,
                       rotator* centre_encoder, rotator* outer_encoder);
int calculate_angles(float x, float y, float *theta1, float *theta2);

static void print_scaled_cm(int value_scaled) {
    int integer_part = value_scaled / SCALE_FACTOR;
    int fractional_part = (value_scaled % SCALE_FACTOR) / 10; // One decimal place
    printf("%d.%d cm", integer_part, fractional_part);
}

static void print_float_as_int_decimal(float value) {
    int sign = (value < 0) ? -1 : 1;
    value = value * sign;

    int integer_part = (int)value;
    int decimal_part = (int)((value - integer_part) * 10); // One decimal place

    if (sign == -1) {
        printf("-%d.%d", integer_part, decimal_part);
    } else {
        printf("%d.%d", integer_part, decimal_part);
    }
}

static void move_servo_by_angle(float angle, servo_motor* servo, rotator* encoder) {
    rotate(angle, *servo, encoder);
}

static void move_electromagnet(float theta1, float theta2,
                               servo_motor* servo_centre, servo_motor* servo_outer,
                               rotator* centre_encoder, rotator* outer_encoder) {

    // Move the centre (theta1) first
    printf("moving motor centre (theta1)\n");
    move_servo_by_angle(theta1, servo_centre, centre_encoder);
    timer_delay_us(100000);

    // Then move the outer (theta2)
    printf("moving motor outer (theta2)\n");
    move_servo_by_angle(theta2, servo_outer, outer_encoder);
    timer_delay_us(100000);
}

// Move a piece by going from initial to source, pick piece, source to destination, drop piece, and return home.
static void move_piece(float src_row, float src_col, float dest_row, float dest_col,
                       servo_motor* servo_centre, servo_motor* servo_outer,
                       rotator* centre_encoder, rotator* outer_encoder) {

    // We'll track how much we've moved so we can return to start
    float sum_theta1 = src_row + dest_row;
    float sum_theta2 = src_col + dest_col;
    printf("sum_theta1, %d", (int)sum_theta1);
    printf("sum_theta2, %d", (int)sum_theta2);

    // Move to source
    printf("Moving to source position: (%d, %d) (", (int)src_row, (int)src_col);
    move_electromagnet(src_row, src_col, servo_centre, servo_outer, centre_encoder, outer_encoder);

    timer_delay_us(100000);

    // Activate electromagnet
    printf("Activating electromagnet at source position.\n");
    set_electromagnet(1);
    timer_delay_us(3000000);

    // Move to destination
    printf("Moving to destination position: (%d, %d) (", (int)dest_row, (int)dest_col);
    move_electromagnet(dest_row, dest_row, servo_centre, servo_outer, centre_encoder, outer_encoder);
    timer_delay_us(100000);

    // Deactivate electromagnet
    printf("Deactivating electromagnet at destination position.\n");
    set_electromagnet(0);
    timer_delay_us(100000);

    printf("Returning to initial position.\n");

    move_servo_by_angle(80, servo_outer, outer_encoder);
    timer_delay_us(100000);
    move_servo_by_angle(50, servo_centre, centre_encoder);
    timer_delay_us(100000);
}

int main(void) {
    interrupts_init();
    uart_init();

    electromagnet_init(GPIO_PB2);
    timer_delay_us(100000);

    rotator* centre_encoder = rotary_init(GPIO_PB1, GPIO_PD14);
    rotator* outer_encoder = rotary_init(GPIO_PC1, GPIO_PD10);

    servo_motor servo_centre;
    servo_motor servo_outer;

    servo_centre.pin = GPIO_PB4;
    servo_centre.type = CENTRE;
    servo_centre.rotation = FULL_ROTATION_CENTRE;

    servo_outer.pin = GPIO_PB3;
    servo_outer.type = OUTER;
    servo_outer.rotation = FULL_ROTATION_OUTER;

    servo_init(servo_outer);
    servo_init(servo_centre);

    interrupts_global_enable();

    move_piece(-30.0, 0.0, -50.0, -50.0, &servo_centre, &servo_outer, centre_encoder, outer_encoder);
    // timer_delay_us(100000);
    // move_piece(45.0, 0.0, 90.0, 90.0, &servo_centre, &servo_outer, centre_encoder, outer_encoder);

    while(true) {}
    return 0;
}

























// #include "uart.h"
// #include "mymodule.h"
// #include "servo.h"
// #include "gpio.h"
// #include "ik.h"
// #include "printf.h"
// #include "electromagnet.h"
// #include "timer.h"
// #include "interrupts.h"
// #include "rotaryencoder.h"

// void main(void) {
//     interrupts_init();
//     uart_init();
//     rotator* centre_encoder = rotary_init(GPIO_PB1, GPIO_PD14); // CENTRE MAKE SURE TO UPDATE IN ALL FUTURE CALLS!
//     rotator* outer_encoder = rotary_init(GPIO_PC1, GPIO_PD10); // OUTER
//     servo_motor servo_centre;
//     servo_motor servo_outer;

//     servo_centre.pin = GPIO_PB4;
//     servo_centre.type = CENTRE;
//     servo_centre.rotation = FULL_ROTATION_CENTRE;

//     servo_outer.pin = GPIO_PB3;
//     servo_outer.type = OUTER;
//     servo_outer.rotation = FULL_ROTATION_OUTER;

//     servo_init(servo_outer);
//     servo_init(servo_centre);

//     interrupts_global_enable();


//     float theta1 = 0.0;
//     float theta2 = 0.0;
//     calculate_angles(0.5, 0.5, &theta1, &theta2);
//     int angle = theta1;
//     int angle2 = theta2;
//     printf("%d, theta1, %d theta2 ", angle, angle2);

//     // rotate(-45, servo_centre, centre_encoder);
//     // rotate(-45, servo_outer, outer_encoder);
//     rotate(45, servo_centre, centre_encoder);
//     // rotate(45, servo_outer, outer_encoder);


//     while(true) {}


//     say_hello("CS107e");
// }