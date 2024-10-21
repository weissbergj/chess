#include "timer.h"

void timer_init() {}

void timer_delay(int secs) {
    timer_delay_ms(secs * 1000);
}