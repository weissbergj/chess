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

