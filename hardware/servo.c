/* File: servo.c
 * Program for controlling MG996R servo motors
 */

#include "gpio.h"
#include "timer.h"
#include "servo.h"
#include "printf.h"
#include "rotaryencoder.h"

#define TOTAL_PULSES 20

void servo_init (servo_motor servo) {
  gpio_set_output(servo.pin);
  return;
}

void rotate(float angle, servo_motor servo, rotator* encoder) {
  int direction;

  if (angle < 0) {
    direction = COUNTER_CLOCKWISE;
    angle *= (-1);
  } else {
    direction = CLOCKWISE;
  }

  unsigned int total_pulses = angle / 360 * TOTAL_PULSES;
  printf("%d, ticks \n", total_pulses);

  while (encoder->pulses <= total_pulses) {
    gpio_write(servo.pin, 1);
    timer_delay_us(direction);
    gpio_write(servo.pin, 0);
    timer_delay_us(PERIOD - direction);
  }
}