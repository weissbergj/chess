/* File: servo.c
 * Program for controlling MG996R servo motors
 */

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
    // // Enforce limits before moving
    // if ((servo.type == OUTER && (angle > 200.0f || angle < -200.0f)) ||
    //     (servo.type == CENTRE && (angle > 200.0f || angle < -200.0f))) {
    //     printf("Error: Angle %d exceeds limits for servo type %d\n", (int)angle, servo.type);
    //     return;
    // }

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
        //   printf("target < pulses: %d\n", target_pulses);
        //   printf("current < pulses: %d\n", encoder->pulses);
            gpio_write(servo.pin, 1);
            timer_delay_us(direction);
            gpio_write(servo.pin, 0);
            timer_delay_us(PERIOD - direction);
        }
    } else {
        // COUNTER_CLOCKWISE
        int target_pulses = (start_pulses - total_pulses);
        // printf("target pulses: %d\n", target_pulses);
        while (encoder->pulses > target_pulses) {
        //   printf("target > pulses: %d\n", target_pulses);
        //   printf("current > pulses: %d\n", encoder->pulses);
            gpio_write(servo.pin, 1);
            timer_delay_us(direction);
            gpio_write(servo.pin, 0);
            timer_delay_us(PERIOD - direction);
        }
    }
}