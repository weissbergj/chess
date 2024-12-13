/* Program: electromagnet.c
 * Controls electromagnet
 */

#include "electromagnet.h"

static gpio_id_t ELECTROMAGNET_PIN;

void electromagnet_init(gpio_id_t pin) {
  gpio_set_output(pin);
  ELECTROMAGNET_PIN = pin;
  gpio_write(ELECTROMAGNET_PIN, 0);
  return;
}

void set_electromagnet(const int setting) {
  gpio_write(ELECTROMAGNET_PIN, setting);
}
