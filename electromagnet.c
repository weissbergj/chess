#include "electromagnet.h"
#include "gpio.h"
#include "timer.h"

// Define GPIO pins for rotary encoder
gpio_id_t rotary[3] = {GPIO_PB8, GPIO_PB9, GPIO_PB5}; // PB8 = A, PB9 = B, PB5 = Button

// Current position of the electromagnet on the chessboard
static int current_row = -1;
static int current_col = -1;

// Define the size of each square on the chessboard in centimeters
#define SQUARE_SIZE_CM 5.0
#define TICKS_PER_REVOLUTION 360 // Adjust based on encoder's specifications

// Function to initialize the rotary encoder
void init_rotary_encoder(void) {
    for (int i = 0; i < 3; i++) {
        gpio_set_input(rotary[i]);         // Configure rotary pins as inputs
        gpio_activate_pullup(rotary[i]);    // Enable pull-up resistors for rotary pins
    }
}

// Function to read the rotary encoder and return the count of ticks
int read_rotary_encoder(void) {
    static int count = 0; // Initialize the count of ticks
    int prevA = gpio_read(rotary[0]); // Read initial state of A pin
    int prevB = gpio_read(rotary[1]); // Read initial state of B pin

    while (1) {
        int A = gpio_read(rotary[0]); // Read current state of A pin
        int B = gpio_read(rotary[1]); // Read current state of B pin

        // Check if the rotary encoder position has changed
        if (A != prevA || B != prevB) {
            if (A != prevA) {
                if (B != A) count--; // Clockwise rotation decreases count
                else count++;         // Counter-clockwise rotation increases count
            }

            // Wrap around the count if necessary
            if (count > 9959) count = 0;   
            else if (count < 0) count = 9959;

            prevA = A; // Update previous state of A
            prevB = B; // Update previous state of B
        }

        // Add a small delay to avoid excessive polling
        timer_delay_us(100);
    }

    return count; // Return the current count of ticks
}

// Function to map chessboard positions to electromagnet control signals
int get_electromagnet_pin(int row, int col) {
    return row * 8 + col;  // Example mapping from chessboard position to GPIO pin
}

// Function to convert physical coordinates (cm) to chessboard coordinates (1-8)
void convert_coordinates_to_chessboard(double x_cm, double y_cm, int *row, int *col) {
    // Calculate the column (x-axis)
    *col = (int)(x_cm / SQUARE_SIZE_CM);
    // Calculate the row (y-axis)
    *row = 7 - (int)(y_cm / SQUARE_SIZE_CM);  // Invert row for chessboard (0 is the bottom)
    
    // Ensure the values are within bounds
    if (*col < 0) *col = 0;
    if (*col > 7) *col = 7;
    if (*row < 0) *row = 0;
    if (*row > 7) *row = 7;
}

// Function to move the sliding electromagnet to a specific chessboard position
void move_electromagnet(int row, int col) {
    if (current_row != -1 && current_col != -1) {
        // Logic to move the electromagnet from the current position to the new position
        // This could involve controlling motors or servos to slide the electromagnet
    }

    // Update the current position of the electromagnet
    current_row = row;
    current_col = col;
}

// Function to control the electromagnet based on rotary encoder input
void control_electromagnet_with_encoder(void) {
    init_rotary_encoder(); // Initialize the rotary encoder

    while (1) {
        int count = read_rotary_encoder(); // Read the rotary encoder count

        // Convert the count to chessboard coordinates
        int target_row = count / 8; // Convert count to target row (0-7)
        int target_col = count % 8; // Convert count to target column (0-7)

        // Move the electromagnet to the target position
        move_electromagnet(target_row, target_col);

        // Add a delay to avoid excessive movement
        timer_delay_ms(100);
    }
}

// Function to initialize the electromagnet system (if needed)
void initialize_electromagnets(void) {
    // Initialization code for the electromagnets, if necessary
}