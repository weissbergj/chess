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

//     rotate(180, servo_centre, centre_encoder);
//     rotate(90, servo_outer, outer_encoder);


//     while(true) {}


//     say_hello("CS107e");
// }



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
    rotator* outer_encoder = rotary_init(GPIO_PC1, GPIO_PD10);

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







// #define PULSES_PER_360_CENTER 1000  // Example value, adjust based on your servo specifications
// #define PULSES_PER_360_OUTER 1000  // Example value, adjust based on your outer servo specifications
// #define MAX_CENTER_ANGLE 180         // Maximum angle for the center servo
// #define MAX_OUTER_ANGLE 180          // Maximum angle for the outer servo


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

// // Function prototype
// void move_and_activate_electromagnet(float x_cm, float y_cm, servo_motor* servo_centre, servo_motor* servo_outer, rotator* encoder);

// void main(void) {
//     interrupts_init();
//     uart_init();
//     rotator* encoder = rotary_init(GPIO_PB1, GPIO_PD14);
//     servo_motor servo_centre;
//     servo_motor servo_outer;

//     servo_centre.pin = GPIO_PB3;
//     servo_centre.type = CENTRE;
//     servo_centre.rotation = FULL_ROTATION_CENTRE;

//     servo_outer.pin = GPIO_PB4;
//     servo_outer.type = OUTER;
//     servo_outer.rotation = FULL_ROTATION_OUTER;

//     servo_init(servo_outer);
//     servo_init(servo_centre);

//     interrupts_global_enable();

//    printf("Calling move_and_activate_electromagnet with x_cm = %f, y_cm = %f\n", 0.5, 0.5);
//     move_and_activate_electromagnet(0.5, 0.5, &servo_centre, &servo_outer, encoder);

//     while(true) {}

//     say_hello("CS107e");
// }

// void move_and_activate_electromagnet(float x_cm, float y_cm, servo_motor* servo_centre, servo_motor* servo_outer, rotator* encoder) {
//     float theta1 = 0.0;
//     float theta2 = 0.0;

//     // Print input values
//     printf("Input coordinates: x_cm = %f, y_cm = %f\n", x_cm, y_cm);

//     // Calculate the required angles using inverse kinematics
//     calculate_angles(x_cm, y_cm, &theta1, &theta2);
//     printf("Calculated angles: theta1 = %f, theta2 = %f\n", theta1, theta2);

//     // Ensure angles are within limits
//     // if (theta1 < 0 || theta1 > MAX_CENTER_ANGLE || theta2 < 0 || theta2 > MAX_OUTER_ANGLE) {
//     //     printf("Error: Angles out of range. theta1: %f, theta2: %f\n", theta1, theta2);
//     //     return;
//     // }

//     // Turn on the electromagnet before moving
//     set_electromagnet(1);

//     // Move the servos to the calculated angles
//     move_servo_to_angle(theta1, servo_centre, encoder, PULSES_PER_360_CENTER);
//     move_servo_to_angle(theta2, servo_outer, encoder, PULSES_PER_360_OUTER);

//     // Movement complete, turn off the electromagnet
//     set_electromagnet(0);
// }



// TO DO

// Write a script that moves the outer arm no more than 180 degrees at all times and the 
// lower moto no more than 360 degrees at all times to prevent things from getting broken. 

// make sure we properly count angles based on number of rotary clicks (we have two rotary encoders
// if you recall one for center one for outer motor arms). then create a function that maps 
// to square centimeters on a board and based on this mapping turns on the electromagnet 
// when moving and hten turns off when the move is completed. 




































// 
// CURRENT WORKING VERSION


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
// #define PULSES_PER_360_CENTER 20  // Adjust based on your servo specifications
// #define PULSES_PER_360_OUTER 20   // Adjust based on your outer servo specifications
// #define MAX_CENTER_ANGLE 360         // Maximum angle for the center servo
// #define MAX_OUTER_ANGLE 180          // Maximum angle for the outer servo
// #define BOARD_SIZE 8
// #define SQUARE_SIZE_CM 5            // Each square is 5 cm
// #define SCALE_FACTOR 100            // Scale to convert cm to integer

// // Initial position (home position)
// #define INITIAL_X_CM 1
// #define INITIAL_Y_CM 1

// // Structure to hold chessboard positions
// typedef struct {
//     int row;       // 0 to 7
//     int col;       // 0 to 7
//     int x_scaled;  // Scaled by SCALE_FACTOR (e.g., 5 cm -> 500)
//     int y_scaled;
// } ChessPosition;

// // Function prototypes
// void move_electromagnet(int x_scaled, int y_scaled, servo_motor* servo_centre, servo_motor* servo_outer, rotator* encoder);
// void move_piece(int src_row, int src_col, int dest_row, int dest_col, servo_motor* servo_centre, servo_motor* servo_outer, rotator* encoder);
// int chess_to_scaled_cm(int row, int col, int* x_scaled, int* y_scaled);
// void print_scaled_cm(int value_scaled);
// void print_float_as_int_decimal(float value);
// void move_servo_to_angle(int angle, servo_motor* servo, rotator* encoder, int pulses_per_360);
// int calculate_angles(float x, float y, float *theta1, float *theta2); // Ensure this is declared in your headers

// // Custom print function to display scaled integers as floating-point equivalents
// void print_scaled_cm(int value_scaled) {
//     int integer_part = value_scaled / SCALE_FACTOR;
//     int fractional_part = (value_scaled % SCALE_FACTOR) / 10; // One decimal place
//     printf("%d.%d cm", integer_part, fractional_part);
// }

// // Helper function to print float values as "int.decimals"
// void print_float_as_int_decimal(float value) {
//     int integer_part = (int)value;
//     int decimal_part = (int)((value - integer_part) * 10); // One decimal place
//     printf("%d.%d", integer_part, decimal_part);
// }

// // Function to map chessboard position to scaled coordinates
// int chess_to_scaled_cm(int row, int col, int* x_scaled, int* y_scaled) {
//     if (col < 0 || col >= BOARD_SIZE || row < 0 || row >= BOARD_SIZE) {
//         // Handle invalid positions appropriately
//         printf("Invalid chess position: (%d, %d)\n", row, col);
//         return -1;
//     }

//     *x_scaled = col * SQUARE_SIZE_CM * SCALE_FACTOR + (SQUARE_SIZE_CM / 2) * SCALE_FACTOR;
//     *y_scaled = row * SQUARE_SIZE_CM * SCALE_FACTOR + (SQUARE_SIZE_CM / 2) * SCALE_FACTOR;

//     return 0;
// }

// // Updated move_electromagnet function
// void move_electromagnet(int x_scaled, int y_scaled, servo_motor* servo_centre, servo_motor* servo_outer, rotator* encoder) {
//     float theta1 = 0.0f;
//     float theta2 = 0.0f;

//     // Print input values
//     printf("Input coordinates: x_cm = ");
//     print_scaled_cm(x_scaled);
//     printf(", y_cm = ");
//     print_scaled_cm(y_scaled);
//     printf("\n");

//     // Calculate the required angles using inverse kinematics
//     calculate_angles(x_scaled / (float)SCALE_FACTOR, y_scaled / (float)SCALE_FACTOR, &theta1, &theta2);
    
//     // Print calculated angles without using %f
//     printf("Calculated angles: theta1 = ");
//     print_float_as_int_decimal(theta1);
//     printf(", theta2 = ");
//     print_float_as_int_decimal(theta2);
//     printf("\n");

//     // Note: Angle limit checks are currently disabled

//     // Convert angles to integers for servo control
//     int angle1 = (int)theta1;
//     int angle2 = (int)theta2;

//     // Move the servos to the calculated angles
//     move_servo_to_angle(angle1, servo_centre, encoder, PULSES_PER_360_CENTER);
//     move_servo_to_angle(angle2, servo_outer, encoder, PULSES_PER_360_OUTER);
// }

// // Function to move a chess piece from source to destination
// void move_piece(int src_row, int src_col, int dest_row, int dest_col, servo_motor* servo_centre, servo_motor* servo_outer, rotator* encoder) {
//     int src_x, src_y, dest_x, dest_y;

//     // chess_to_scaled_cm(src_row, src_col, &src_x, &src_y);
//     // chess_to_scaled_cm(dest_row, dest_col, &dest_x, &dest_y);

//     // Convert chess positions to scaled physical coordinates
//     if (chess_to_scaled_cm(src_row, src_col, &src_x, &src_y) != 0) {
//         // Handle invalid source position
//         return;
//     }
//     if (chess_to_scaled_cm(dest_row, dest_col, &dest_x, &dest_y) != 0) {
//         // Handle invalid destination position
//         return;
//     }

//     // Move to source position without electromagnet
//     printf("Moving to source position: (%d, %d) (", src_row, src_col);
//     print_scaled_cm(src_x);
//     printf(", ");
//     print_scaled_cm(src_y);
//     printf(")\n");
//     move_electromagnet(src_x, src_y, servo_centre, servo_outer, encoder);

//     // Activate electromagnet to pick up the piece
//     printf("Activating electromagnet at source position.\n");
//     set_electromagnet(1);

//     // Move to destination position with electromagnet
//     printf("Moving to destination position: (%d, %d) (", dest_row, dest_col);
//     print_scaled_cm(dest_x);
//     printf(", ");
//     print_scaled_cm(dest_y);
//     printf(")\n");
//     move_electromagnet(dest_x, dest_y, servo_centre, servo_outer, encoder);

//     // Deactivate electromagnet to place the piece
//     printf("Deactivating electromagnet at destination position.\n");
//     set_electromagnet(0);

//     // Return to initial position
//     printf("Returning to initial position.\n");
//     move_electromagnet(INITIAL_X_CM * SCALE_FACTOR, INITIAL_Y_CM * SCALE_FACTOR, servo_centre, servo_outer, encoder);
// }

// // Function to move servos to a specific angle with controlled speed
// void move_servo_to_angle(int angle, servo_motor* servo, rotator* encoder, int pulses_per_360) {
//     // Calculate the total pulses needed for the desired angle
//     int total_pulses = (angle * pulses_per_360) / 360;

//     // Move the servo
//     rotate(angle, *servo, encoder);

//     // Optional: Add a delay to control speed
//     // timer_delay_ms(500);  // Delay for 500 milliseconds
// }

// int main(void) {
//     interrupts_init();
//     uart_init();
//     rotator* encoder = rotary_init(GPIO_PB1, GPIO_PD14);
//     servo_motor servo_centre;
//     servo_motor servo_outer;

//     servo_centre.pin = GPIO_PB3;
//     servo_centre.type = CENTRE;
//     servo_centre.rotation = FULL_ROTATION_CENTRE;

//     servo_outer.pin = GPIO_PB4;
//     servo_outer.type = OUTER;
//     servo_outer.rotation = FULL_ROTATION_OUTER;

//     servo_init(servo_outer);
//     servo_init(servo_centre);

//     interrupts_global_enable();

//     // Example move: Move piece from (0,0) to (1,1)
//     move_piece(2, 2, 3, 3, &servo_centre, &servo_outer, encoder);

//     while(true) {}

//     say_hello("CS107e");
//     return 0;
// }













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

// // Define constants (adjust as per your servo specs)
// #define PULSES_PER_360_CENTER 20
// #define PULSES_PER_360_OUTER 20
// #define MAX_CENTER_ANGLE 360
// #define MAX_OUTER_ANGLE 180
// #define BOARD_SIZE 8
// #define SQUARE_SIZE_CM 5
// #define SCALE_FACTOR 100
// #define INITIAL_X_CM 1
// #define INITIAL_Y_CM 1

// // Forward declarations
// static float get_servo_angle(rotator* encoder, int pulses_per_360);
// static void move_servo_to_angle(float target_angle, servo_motor* servo, rotator* encoder, int pulses_per_360);
// static int enforce_angle_limits(float* theta1, float* theta2);

// int chess_to_scaled_cm(int row, int col, int* x_scaled, int* y_scaled);
// void print_scaled_cm(int value_scaled);
// void print_float_as_int_decimal(float value);
// void move_electromagnet(int x_scaled, int y_scaled, servo_motor* servo_centre, servo_motor* servo_outer, rotator* centre_encoder, rotator* outer_encoder);
// void move_piece(int src_row, int src_col, int dest_row, int dest_col, servo_motor* servo_centre, servo_motor* servo_outer, rotator* centre_encoder, rotator* outer_encoder);

// // Custom print function to display scaled integers as floating-point equivalents
// void print_scaled_cm(int value_scaled) {
//     int integer_part = value_scaled / SCALE_FACTOR;
//     int fractional_part = (value_scaled % SCALE_FACTOR) / 10; // One decimal place
//     printf("%d.%d cm", integer_part, fractional_part);
// }

// // Helper function to print float values as "int.decimals"
// void print_float_as_int_decimal(float value) {
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

// // Map chessboard position to scaled coordinates
// int chess_to_scaled_cm(int row, int col, int* x_scaled, int* y_scaled) {
//     if (col < 0 || col >= BOARD_SIZE || row < 0 || row >= BOARD_SIZE) {
//         printf("Invalid chess position: (%d, %d)\n", row, col);
//         return -1;
//     }

//     *x_scaled = col * SQUARE_SIZE_CM * SCALE_FACTOR + (SQUARE_SIZE_CM / 2) * SCALE_FACTOR;
//     *y_scaled = row * SQUARE_SIZE_CM * SCALE_FACTOR + (SQUARE_SIZE_CM / 2) * SCALE_FACTOR;
//     return 0;
// }

// // Get current servo angle from encoder
// static float get_servo_angle(rotator* encoder, int pulses_per_360) {
//     return ((float)encoder->pulses / pulses_per_360) * 360.0f;
// }

// // Ensure angles are within mechanical limits
// static int enforce_angle_limits(float* theta1, float* theta2) {
//     int valid = 1;
//     if (*theta1 < 0 || *theta1 > MAX_CENTER_ANGLE) {
//         printf("Warning: theta1 out of range: ");
//         print_float_as_int_decimal(*theta1);
//         printf("\n");
//         valid = 0;
//     }
//     if (*theta2 < 0 || *theta2 > MAX_OUTER_ANGLE) {
//         printf("Warning: theta2 out of range: ");
//         print_float_as_int_decimal(*theta2);
//         printf("\n");
//         valid = 0;
//     }
//     return valid;
// }

// // Move servo to a specific absolute angle
// static void move_servo_to_angle(float target_angle, servo_motor* servo, rotator* encoder, int pulses_per_360) {
//     float current_angle = get_servo_angle(encoder, pulses_per_360);
//     float angle_diff = target_angle - current_angle;
//     // Use rotate() to move by the difference
//     rotate(angle_diff, *servo, encoder);
//     // Optional delay
//     timer_delay_ms(500);
// }

// // Move electromagnet to (x_scaled, y_scaled)
// void move_electromagnet(int x_scaled, int y_scaled,
//                         servo_motor* servo_centre, servo_motor* servo_outer,
//                         rotator* centre_encoder, rotator* outer_encoder) {
//     float theta1 = 0.0f, theta2 = 0.0f;

//     printf("Input coordinates: x_cm = ");
//     print_scaled_cm(x_scaled);
//     printf(", y_cm = ");
//     print_scaled_cm(y_scaled);
//     printf("\n");

//     // Compute IK
//     calculate_angles(x_scaled / (float)SCALE_FACTOR, y_scaled / (float)SCALE_FACTOR, &theta1, &theta2);

//     printf("Calculated angles: theta1 = ");
//     print_float_as_int_decimal(theta1);
//     printf(", theta2 = ");
//     print_float_as_int_decimal(theta2);
//     printf("\n");

//     // Check mechanical limits
//     if (!enforce_angle_limits(&theta1, &theta2)) {
//         printf("Angles out of range. Move not executed.\n");
//         return;
//     }

//     // Move each servo to its required angle
//     move_servo_to_angle(theta1, servo_centre, centre_encoder, PULSES_PER_360_CENTER);
//     move_servo_to_angle(theta2, servo_outer, outer_encoder, PULSES_PER_360_OUTER);
// }

// // Move a piece from (src_row, src_col) to (dest_row, dest_col)
// void move_piece(int src_row, int src_col, int dest_row, int dest_col,
//                 servo_motor* servo_centre, servo_motor* servo_outer,
//                 rotator* centre_encoder, rotator* outer_encoder) {
//     int src_x, src_y, dest_x, dest_y;

//     if (chess_to_scaled_cm(src_row, src_col, &src_x, &src_y) != 0) return;
//     if (chess_to_scaled_cm(dest_row, dest_col, &dest_x, &dest_y) != 0) return;

//     printf("Moving to source position: (%d, %d) (", src_row, src_col);
//     print_scaled_cm(src_x);
//     printf(", ");
//     print_scaled_cm(src_y);
//     printf(")\n");
//     move_electromagnet(src_x, src_y, servo_centre, servo_outer, centre_encoder, outer_encoder);

//     printf("Activating electromagnet at source position.\n");
//     set_electromagnet(1);

//     printf("Moving to destination position: (%d, %d) (", dest_row, dest_col);
//     print_scaled_cm(dest_x);
//     printf(", ");
//     print_scaled_cm(dest_y);
//     printf(")\n");
//     move_electromagnet(dest_x, dest_y, servo_centre, servo_outer, centre_encoder, outer_encoder);

//     printf("Deactivating electromagnet at destination position.\n");
//     set_electromagnet(0);

//     printf("Returning to initial position.\n");
//     int init_x = INITIAL_X_CM * SCALE_FACTOR;
//     int init_y = INITIAL_Y_CM * SCALE_FACTOR;
//     move_electromagnet(init_x, init_y, servo_centre, servo_outer, centre_encoder, outer_encoder);
// }

// int main(void) {
//     interrupts_init();
//     uart_init();

//     // Initialize separate encoders for each joint if needed
//     // If you have a single encoder, consider how to differentiate center/outer servo
//     // For demonstration, assume one encoder per servo joint.
//     // If you only have one encoder, you need to clarify how you track each servo.
//     // For now, assume separate encoders:
//     rotator* centre_encoder = rotary_init(GPIO_PB1, GPIO_PD14);
//     rotator* outer_encoder = rotary_init(GPIO_PB2, GPIO_PD15);

//     servo_motor servo_centre;
//     servo_motor servo_outer;

//     servo_centre.pin = GPIO_PB3;
//     servo_centre.type = CENTRE;
//     servo_centre.rotation = FULL_ROTATION_CENTRE;

//     servo_outer.pin = GPIO_PB4;
//     servo_outer.type = OUTER;
//     servo_outer.rotation = FULL_ROTATION_OUTER;

//     servo_init(servo_outer);
//     servo_init(servo_centre);

//     interrupts_global_enable();

//     // Example move: Move piece from (2,2) to (3,3)
//     move_piece(2, 2, 3, 3, &servo_centre, &servo_outer, centre_encoder, outer_encoder);

//     while(true) {}

//     return 0;
// }













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
// #define PULSES_PER_360_CENTER 20   // Adjust based on your servo specifications
// #define PULSES_PER_360_OUTER 20    // Adjust based on your outer servo specifications
// #define MAX_CENTER_ANGLE 360       // Maximum angle for the center servo
// #define MAX_OUTER_ANGLE 180        // Maximum angle for the outer servo
// #define BOARD_SIZE 8
// #define SQUARE_SIZE_CM 5           // Each square is 5 cm
// #define SCALE_FACTOR 100           // Scale to convert cm to integer

// // Initial position (home position)
// #define INITIAL_X_CM 1
// #define INITIAL_Y_CM 1

// // Function prototypes
// static float get_current_angle(rotator* encoder, int pulses_per_360);
// static void move_to_position(float x, float y,
//                              servo_motor* servo_centre, servo_motor* servo_outer,
//                              rotator* centre_encoder, rotator* outer_encoder);
// int chess_to_scaled_cm(int row, int col, int* x_scaled, int* y_scaled);
// void print_scaled_cm(int value_scaled);
// void print_float_as_int_decimal(float value);
// void move_servo_to_angle(float angle, servo_motor* servo, rotator* encoder, int pulses_per_360);
// int calculate_angles(float x, float y, float *theta1, float *theta2);
// void move_electromagnet(int x_scaled, int y_scaled,
//                         servo_motor* servo_centre, servo_motor* servo_outer,
//                         rotator* centre_encoder, rotator* outer_encoder);
// void move_piece(int src_row, int src_col, int dest_row, int dest_col,
//                 servo_motor* servo_centre, servo_motor* servo_outer,
//                 rotator* centre_encoder, rotator* outer_encoder);

// // Custom print function to display scaled integers as floating-point equivalents
// void print_scaled_cm(int value_scaled) {
//     int integer_part = value_scaled / SCALE_FACTOR;
//     int fractional_part = (value_scaled % SCALE_FACTOR) / 10; // One decimal place
//     printf("%d.%d cm", integer_part, fractional_part);
// }

// // Helper function to print float values as "int.decimals"
// void print_float_as_int_decimal(float value) {
//     int sign = (value < 0) ? -1 : 1;
//     value *= sign;

//     int integer_part = (int)value;
//     int decimal_part = (int)((value - integer_part) * 10); // One decimal place

//     if (sign == -1) {
//         printf("-%d.%d", integer_part, decimal_part);
//     } else {
//         printf("%d.%d", integer_part, decimal_part);
//     }
// }

// // Function to map chessboard position to scaled coordinates
// int chess_to_scaled_cm(int row, int col, int* x_scaled, int* y_scaled) {
//     if (col < 0 || col >= BOARD_SIZE || row < 0 || row >= BOARD_SIZE) {
//         printf("Invalid chess position: (%d, %d)\n", row, col);
//         return -1;
//     }

//     *x_scaled = col * SQUARE_SIZE_CM * SCALE_FACTOR + (SQUARE_SIZE_CM / 2) * SCALE_FACTOR;
//     *y_scaled = row * SQUARE_SIZE_CM * SCALE_FACTOR + (SQUARE_SIZE_CM / 2) * SCALE_FACTOR;

//     return 0;
// }

// // Helper function to get current angle of a servo from its encoder
// static float get_current_angle(rotator* encoder, int pulses_per_360) {
//     return ((float)encoder->pulses / pulses_per_360) * 360.0f;
// }

// // Helper function to move from current position to target position (x,y in cm)
// static void move_to_position(float x, float y,
//                              servo_motor* servo_centre, servo_motor* servo_outer,
//                              rotator* centre_encoder, rotator* outer_encoder) 
// {
//     float theta_target1, theta_target2;
//     if (calculate_angles(x, y, &theta_target1, &theta_target2) != 0) {
//         printf("Could not calculate angles for (%.2f, %.2f)\n", x, y);
//         return;
//     }

//     printf("Calculated angles: theta1 = ");
//     print_float_as_int_decimal(theta_target1);
//     printf(", theta2 = ");
//     print_float_as_int_decimal(theta_target2);
//     printf("\n");

//     float theta_current1 = get_current_angle(centre_encoder, PULSES_PER_360_CENTER);
//     float theta_current2 = get_current_angle(outer_encoder, PULSES_PER_360_OUTER);

//     float delta_theta1 = theta_target1 - theta_current1;
//     float delta_theta2 = theta_target2 - theta_current2;

//     // Move the servos by the calculated difference
//     move_servo_to_angle(delta_theta1, servo_centre, centre_encoder, PULSES_PER_360_CENTER);
//     move_servo_to_angle(delta_theta2, servo_outer, outer_encoder, PULSES_PER_360_OUTER);
// }

// // Updated move_electromagnet function to call move_to_position
// void move_electromagnet(int x_scaled, int y_scaled,
//                         servo_motor* servo_centre, servo_motor* servo_outer,
//                         rotator* centre_encoder, rotator* outer_encoder) 
// {
//     float x_cm = x_scaled / (float)SCALE_FACTOR;
//     float y_cm = y_scaled / (float)SCALE_FACTOR;

//     printf("Input coordinates: x_cm = ");
//     print_scaled_cm(x_scaled);
//     printf(", y_cm = ");
//     print_scaled_cm(y_scaled);
//     printf(" (%.2f cm, %.2f cm)\n", x_cm, y_cm);

//     // Call the helper function to handle IK and current angles
//     move_to_position(x_cm, y_cm, servo_centre, servo_outer, centre_encoder, outer_encoder);
// }

// // Function to move a chess piece from source to destination
// void move_piece(int src_row, int src_col, int dest_row, int dest_col,
//                 servo_motor* servo_centre, servo_motor* servo_outer,
//                 rotator* centre_encoder, rotator* outer_encoder) 
// {
//     int src_x, src_y, dest_x, dest_y;

//     if (chess_to_scaled_cm(src_row, src_col, &src_x, &src_y) != 0) return;
//     if (chess_to_scaled_cm(dest_row, dest_col, &dest_x, &dest_y) != 0) return;

//     printf("Moving to source position: (%d, %d) (", src_row, src_col);
//     print_scaled_cm(src_x);
//     printf(", ");
//     print_scaled_cm(src_y);
//     printf(")\n");
//     move_electromagnet(src_x, src_y, servo_centre, servo_outer, centre_encoder, outer_encoder);

//     printf("Activating electromagnet at source position.\n");
//     set_electromagnet(1);

//     printf("Moving to destination position: (%d, %d) (", dest_row, dest_col);
//     print_scaled_cm(dest_x);
//     printf(", ");
//     print_scaled_cm(dest_y);
//     printf(")\n");
//     move_electromagnet(dest_x, dest_y, servo_centre, servo_outer, centre_encoder, outer_encoder);

//     printf("Deactivating electromagnet at destination position.\n");
//     set_electromagnet(0);

//     printf("Returning to initial position.\n");
//     move_electromagnet(INITIAL_X_CM * SCALE_FACTOR, INITIAL_Y_CM * SCALE_FACTOR,
//                        servo_centre, servo_outer, centre_encoder, outer_encoder);
// }

// // Function to move servos by a given angle (float) with controlled speed
// void move_servo_to_angle(float angle, servo_motor* servo, rotator* encoder, int pulses_per_360) {
//     // Here 'angle' is how much we rotate relative to the current angle.
//     // 'rotate(angle, *servo, encoder)' is assumed to handle this increment.

//     rotate(angle, *servo, encoder);

//     // Optional: Add a delay to control speed if desired
//     timer_delay_ms(500);  // Delay for 500 milliseconds
// }

// int main(void) {
//     interrupts_init();
//     uart_init();

//     // Initialize encoders for each joint
//     rotator* centre_encoder = rotary_init(GPIO_PB1, GPIO_PD14);
//     rotator* outer_encoder = rotary_init(GPIO_PB2, GPIO_PD15);

//     servo_motor servo_centre;
//     servo_motor servo_outer;

//     servo_centre.pin = GPIO_PB3;
//     servo_centre.type = CENTRE;
//     servo_centre.rotation = FULL_ROTATION_CENTRE;

//     servo_outer.pin = GPIO_PB4;
//     servo_outer.type = OUTER;
//     servo_outer.rotation = FULL_ROTATION_OUTER;

//     servo_init(servo_outer);
//     servo_init(servo_centre);

//     interrupts_global_enable();

//     // Example move: Move piece from (2,2) to (3,3)
//     move_piece(2, 2, 3, 3, &servo_centre, &servo_outer, centre_encoder, outer_encoder);

//     while(true) {}

//     say_hello("CS107e");
//     return 0;
// }
