#ifndef SERVO_H
#define SERVO_H
#define CLOCKWISE 1000
#define COUNTER_CLOCKWISE 2000
#define NEUTRAL 1500
#define PERIOD 20000
//#define FULL_ROTATION_CENTRE 35500000
#define FULL_ROTATION_CENTRE 100000000
#define FULL_ROTATION_OUTER 120000000
#define TICKS_PER_SEC 24000000

#include "rotaryencoder.h"

typedef struct {
  gpio_id_t pin;
  enum { CENTRE = 0, OUTER } type;
  unsigned long rotation;
} servo_motor;

void servo_init(servo_motor servo);

void rotate(float angle1, servo_motor servo1, rotator* encoder);

#endif


// #ifndef SERVO_H
// #define SERVO_H

// #include "gpio.h"
// #include "rotaryencoder.h"
// #include "timer.h"
// #include "printf.h"

// // Direction pulse widths for servo control (in microseconds)
// #define CLOCKWISE 1000
// #define COUNTER_CLOCKWISE 2000
// #define NEUTRAL 1500
// #define PERIOD 20000

// // Constants representing maximum rotation ticks or timing for different servo types
// // Adjust these based on your servo specifications.
// #define FULL_ROTATION_CENTRE 100000000
// #define FULL_ROTATION_OUTER 120000000

// // Pulses per full 360° rotation (example value, adjust as needed)
// #define PULSES_PER_360_CENTER 20
// #define PULSES_PER_360_OUTER  20

// // Structure defining a servo motor
// typedef struct {
//     gpio_id_t pin;        // The GPIO pin controlling the servo’s signal line
//     enum { CENTRE = 0, OUTER } type; // Type of servo (center or outer)
//     unsigned long rotation; // A value representing how many ticks per full rotation (adjust as needed)
// } servo_motor;

// // Initializes the given servo by setting its control pin as output
// void servo_init(servo_motor servo);

// // Rotates the given servo by the specified angle (in degrees), using the provided encoder for feedback.
// // Positive angle: clockwise direction, negative angle: counter-clockwise.
// // This function uses encoder->pulses and the defined constants to control the servo.
// void rotate(float angle, servo_motor servo, rotator* encoder);

// #endif // SERVO_H
