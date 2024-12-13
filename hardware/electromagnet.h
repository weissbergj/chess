/* Program: electromagnet.h
 * Header file for electromagnet module
 */
#include "gpio.h"

enum { OFF = 0, ON };

void electromagnet_init(gpio_id_t pin);

void set_electromagnet(const int setting);
