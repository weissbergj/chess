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

// Define constants
#define PULSES_PER_360_CENTER 20
#define PULSES_PER_360_OUTER  20
#define MAX_CENTER_ANGLE 360
#define MAX_OUTER_ANGLE 180
#define BOARD_SIZE 8
#define SQUARE_SIZE_CM 5
#define SCALE_FACTOR 100
#define INITIAL_X_CM 1
#define INITIAL_Y_CM 1

// Function prototypes
static int chess_to_scaled_cm(int row, int col, int* x_scaled, int* y_scaled);
static void print_scaled_cm(int value_scaled);
static void print_float_as_int_decimal(float value);
static void move_electromagnet(int x_scaled, int y_scaled, servo_motor* servo_centre, servo_motor* servo_outer, 
                               rotator* centre_encoder, rotator* outer_encoder,
                               float *sum_theta1, float *sum_theta2);
static void move_piece(int src_row, int src_col, int dest_row, int dest_col,
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

// Map chessboard position to scaled coordinates
static int chess_to_scaled_cm(int row, int col, int* x_scaled, int* y_scaled) {
    if (col < 0 || col >= BOARD_SIZE || row < 0 || row >= BOARD_SIZE) {
        printf("Invalid chess position: (%d, %d)\n", row, col);
        return -1;
    }

    *x_scaled = col * SQUARE_SIZE_CM * SCALE_FACTOR + (SQUARE_SIZE_CM / 2) * SCALE_FACTOR;
    *y_scaled = row * SQUARE_SIZE_CM * SCALE_FACTOR + (SQUARE_SIZE_CM / 2) * SCALE_FACTOR;

    return 0;
}

// Move by an angle difference using rotate()
static void move_servo_by_angle(float angle, servo_motor* servo, rotator* encoder) {
    // This will rotate by 'angle' degrees relative to current position.
    rotate(angle, *servo, encoder);
}

// Move electromagnet to a given (x_scaled, y_scaled) by computing IK and rotating servos.
// sum_theta1 and sum_theta2 store the cumulative angles moved so we can return to start by reversing them.
static void move_electromagnet(int x_scaled, int y_scaled,
                               servo_motor* servo_centre, servo_motor* servo_outer,
                               rotator* centre_encoder, rotator* outer_encoder,
                               float *sum_theta1, float *sum_theta2) {
    float theta1 = 0.0f;
    float theta2 = 0.0f;

    printf("Input coordinates: x_cm = ");
    print_scaled_cm(x_scaled);
    printf(", y_cm = ");
    print_scaled_cm(y_scaled);
    printf("\n");

    // Calculate angles using IK
    calculate_angles(x_scaled / (float)SCALE_FACTOR, y_scaled / (float)SCALE_FACTOR, &theta1, &theta2);

    printf("Calculated angles: theta1 = ");
    print_float_as_int_decimal(theta1);
    printf(", theta2 = ");
    print_float_as_int_decimal(theta2);
    printf("\n");

    // Since we're doing relative moves, we just call rotate with these angles
    // This moves from the current position by theta1 and theta2 more degrees
    move_servo_by_angle(theta1, servo_centre, centre_encoder);
    move_servo_by_angle(theta2, servo_outer, outer_encoder);

    // Add these angles to the cumulative sum
    *sum_theta1 += theta1;
    *sum_theta2 += theta2;
}

// Move a piece by going from initial to source, pick piece, source to destination, drop piece, and return home.
static void move_piece(int src_row, int src_col, int dest_row, int dest_col,
                       servo_motor* servo_centre, servo_motor* servo_outer,
                       rotator* centre_encoder, rotator* outer_encoder) {
    int src_x, src_y, dest_x, dest_y;
    if (chess_to_scaled_cm(src_row, src_col, &src_x, &src_y) != 0) return;
    if (chess_to_scaled_cm(dest_row, dest_col, &dest_x, &dest_y) != 0) return;

    // We'll track how much we've moved so we can return to start
    float sum_theta1 = 0.0f;
    float sum_theta2 = 0.0f;

    // Move to source
    printf("Moving to source position: (%d, %d) (", src_row, src_col);
    print_scaled_cm(src_x);
    printf(", ");
    print_scaled_cm(src_y);
    printf(")\n");
    move_electromagnet(src_x, src_y, servo_centre, servo_outer, centre_encoder, outer_encoder, &sum_theta1, &sum_theta2);

    // Activate electromagnet
    printf("Activating electromagnet at source position.\n");
    set_electromagnet(1);

    // Move to destination
    printf("Moving to destination position: (%d, %d) (", dest_row, dest_col);
    print_scaled_cm(dest_x);
    printf(", ");
    print_scaled_cm(dest_y);
    printf(")\n");
    move_electromagnet(dest_x, dest_y, servo_centre, servo_outer, centre_encoder, outer_encoder, &sum_theta1, &sum_theta2);

    // Deactivate electromagnet
    printf("Deactivating electromagnet at destination position.\n");
    set_electromagnet(0);

    // Return to initial position by negating all moves
    // We moved sum_theta1 and sum_theta2 from start, so just rotate back by -sum_theta2 then -sum_theta1
    // We reverse the order to retrace our steps correctly:
    // Last servo moved was outer, then centre. So to go back:
    // move outer by -theta2, then centre by -theta1 in reverse order as needed.

    printf("Returning to initial position.\n");
    // We made moves in order: first theta1 (centre), then theta2 (outer) at each step.
    // To undo: first undo outer moves: rotate(-sum_theta2), then undo centre moves: rotate(-sum_theta1).
    move_servo_by_angle(-sum_theta2, servo_outer, outer_encoder);
    move_servo_by_angle(-sum_theta1, servo_centre, centre_encoder);
}

int main(void) {
    interrupts_init();
    uart_init();

    rotator* centre_encoder = rotary_init(GPIO_PB1, GPIO_PD14);
    rotator* outer_encoder = rotary_init(GPIO_PB2, GPIO_PD15);

    servo_motor servo_centre;
    servo_motor servo_outer;

    servo_centre.pin = GPIO_PB3;
    servo_centre.type = CENTRE;
    servo_centre.rotation = FULL_ROTATION_CENTRE;

    servo_outer.pin = GPIO_PB4;
    servo_outer.type = OUTER;
    servo_outer.rotation = FULL_ROTATION_OUTER;

    servo_init(servo_outer);
    servo_init(servo_centre);

    interrupts_global_enable();

    // Example move: Move piece from (2,2) to (3,3)
    // This will:
    // 1. Move from init to (2,2)
    // 2. Pick piece
    // 3. Move to (3,3)
    // 4. Drop piece
    // 5. Return back to initial position by reversing the ticks
    move_piece(2, 2, 3, 3, &servo_centre, &servo_outer, centre_encoder, outer_encoder);

    while(true) {}

    say_hello("CS107e");
    return 0;
}
