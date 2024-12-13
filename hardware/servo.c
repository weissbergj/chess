// /* File: servo.c
//  * Program for controlling MG996R servo motors
//  */

// #include "gpio.h"
// #include "timer.h"
// #include "servo.h"
// #include "printf.h"
// #include "rotaryencoder.h"

// #define TOTAL_PULSES 23

// void servo_init (servo_motor servo) {
//   gpio_set_output(servo.pin);
//   return;
// }

// void rotate(float angle, servo_motor servo, rotator* encoder) {
//   int direction;

//   if ((servo.type == OUTER && (angle > 170 || angle < -170)) ||
//       (servo.type == CENTRE && (angle > 180 || angle < -180))) {
//     printf("Error: Angle %f exceeds limits for servo type %d\n", angle, servo.type);
//     return;
//   }

//   if (angle < 0) {
//     direction = COUNTER_CLOCKWISE;
//     angle *= (-1);
//   } else {
//     direction = CLOCKWISE;
//   }

//   unsigned int total_pulses = angle / 360 * TOTAL_PULSES;
//   printf("%d, ticks \n", total_pulses);

//   while (encoder->pulses <= total_pulses) {
//     gpio_write(servo.pin, 1);
//     timer_delay_us(direction);
//     gpio_write(servo.pin, 0);
//     timer_delay_us(PERIOD - direction);
//   }
// }

#include "gpio.h"
#include "timer.h"
#include "servo.h"
#include "printf.h"
#include "rotaryencoder.h"

#define TOTAL_PULSES 23

void servo_init(servo_motor servo) {
    gpio_set_output(servo.pin);
    return;
}

void rotate(float angle, servo_motor servo, rotator* encoder) {
    // Enforce limits before moving
    if ((servo.type == OUTER && (angle > 170.0f || angle < -170.0f)) ||
        (servo.type == CENTRE && (angle > 180.0f || angle < -180.0f))) {
        printf("Error: Angle %f exceeds limits for servo type %d\n", angle, servo.type);
        return;
    }

    int direction;
    float abs_angle = angle;
    if (abs_angle < 0) {
        direction = COUNTER_CLOCKWISE;
        abs_angle = -abs_angle;
    } else {
        direction = CLOCKWISE;
    }

    // Capture the starting pulse count
    unsigned int start_pulses = encoder->pulses;

    // Compute how many pulses we need for the given angle
    unsigned int total_pulses = (unsigned int)((abs_angle / 360.0f) * (float)TOTAL_PULSES);
    printf("%d, ticks required for angle %f\n", total_pulses, angle);

    if (direction == CLOCKWISE) {
        unsigned int target_pulses = start_pulses + total_pulses;
        // Move until current pulses reach target
        while (encoder->pulses < target_pulses) {
          printf("target pulses: %d", target_pulses);
          printf("current pulses: %d", encoder->pulses);
            gpio_write(servo.pin, 1);
            timer_delay_us(direction);
            gpio_write(servo.pin, 0);
            timer_delay_us(PERIOD - direction);
        }
    } else {
        // COUNTER_CLOCKWISE
        // Here we assume encoder->pulses will decrease when moving counter-clockwise.
        // If your encoder logic doesn't decrease pulses, you need to fix that in your encoder handler.
        unsigned int target_pulses = (total_pulses > start_pulses) ? 0 : (start_pulses - total_pulses);
        while (encoder->pulses > target_pulses) {
          printf("target pulses: %d", target_pulses);
          printf("current pulses: %d", encoder->pulses);
            gpio_write(servo.pin, 1);
            timer_delay_us(direction);
            gpio_write(servo.pin, 0);
            timer_delay_us(PERIOD - direction);
        }
    }
}





// void rotate(float angle, servo_motor servo, rotator* encoder) {
//     int direction;

//     // Determine direction and make angle positive
//     if (angle < 0) {
//         direction = COUNTER_CLOCKWISE;
//         angle = -angle;
//     } else {
//         direction = CLOCKWISE;
//     }

//     // Convert angle to pulses. If you want to differentiate between center and outer servo, 
//     // use the appropriate pulses_per_360 constant:
//     int pulses_per_360 = (servo.type == CENTRE) ? PULSES_PER_360_CENTER : PULSES_PER_360_OUTER;
//     unsigned int total_pulses = (unsigned int)((angle / 360.0f) * pulses_per_360);

//     printf("%d ticks required\n", total_pulses);

//     // Capture the starting pulse count
//     unsigned int start_pulses = encoder->pulses;

//     // Compute target pulses based on direction
//     unsigned int target_pulses;
//     if (direction == CLOCKWISE) {
//         target_pulses = start_pulses + total_pulses;
//         // Move until current pulses reach or exceed the target
//         while (encoder->pulses < target_pulses) {
//             gpio_write(servo.pin, 1);
//             timer_delay_us(direction);
//             gpio_write(servo.pin, 0);
//             timer_delay_us(PERIOD - direction);
//         }
//     } else {
//         // COUNTER_CLOCKWISE
//         // Ensure we don't go below zero if start_pulses < total_pulses
//         // In practice, you should ensure the servo and encoder never read negative,
//         // but handle it gracefully just in case:
//         target_pulses = (total_pulses > start_pulses) ? 0 : (start_pulses - total_pulses);
//         // Move until current pulses drop below the target
//         while (encoder->pulses > target_pulses) {
//             gpio_write(servo.pin, 1);
//             timer_delay_us(direction);
//             gpio_write(servo.pin, 0);
//             timer_delay_us(PERIOD - direction);
//         }
//     }
// }













// void rotate(float angle, servo_motor servo, rotator* encoder) {
//     int direction;

//     // Determine direction and make angle positive
//     if (angle < 0) {
//         direction = COUNTER_CLOCKWISE;
//         angle = -angle;
//     } else {
//         direction = CLOCKWISE;
//     }

//     // Calculate how many pulses correspond to the desired rotation
//     unsigned int total_pulses = (angle / 360.0f) * TOTAL_PULSES;
//     printf("%d, ticks \n", total_pulses);

//     // Capture the starting pulse count
//     unsigned int start_pulses = encoder->pulses;

//     // Compute target pulses based on direction
//     if (direction == CLOCKWISE) {
//         unsigned int target_pulses = start_pulses + total_pulses;
//         // Move until current pulses reach or exceed the target
//         while (encoder->pulses < target_pulses) {
//             gpio_write(servo.pin, 1);
//             timer_delay_us(direction);
//             gpio_write(servo.pin, 0);
//             timer_delay_us(PERIOD - direction);
//         }
//     } else { // COUNTER_CLOCKWISE
//         unsigned int target_pulses = (total_pulses > start_pulses) ? 0 : (start_pulses - total_pulses);
//         // Move until current pulses drop below the target
//         while (encoder->pulses > target_pulses) {
//             gpio_write(servo.pin, 1);
//             timer_delay_us(direction);
//             gpio_write(servo.pin, 0);
//             timer_delay_us(PERIOD - direction);
//         }
//     }
// }
