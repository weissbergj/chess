/* Program: rotaryencoder.c
 * Determines angles turned by servo
 */

#include "gpio.h"
#include "rotaryencoder.h"
#include "malloc.h"
#include "printf.h"
#include "gpio_interrupt.h"

void activate_pullup(gpio_id_t id);

rotator* rotary_init(gpio_id_t rotate_a, gpio_id_t rotate_b) {
  rotator *r_encoder = malloc(sizeof(*r_encoder));
  r_encoder->rotate_a = rotate_a;
  r_encoder->rotate_b = rotate_b;
  r_encoder->pulses = 0;
  gpio_set_input(rotate_b);
  gpio_set_input(rotate_a);
  activate_pullup(rotate_a);
  activate_pullup(rotate_b);
  config_rotate((void *) r_encoder);
  return r_encoder;
}

void handler(void *aux_data) {
  rotator *r_encoder = (rotator *) aux_data;
  int rotate_b_state = gpio_read(r_encoder->rotate_b);
  gpio_interrupt_clear(r_encoder->rotate_a);

  if (rotate_b_state) {
    r_encoder->pulses--;
  } else {
    r_encoder->pulses++; // SOMETIMES BREAKS... NEED TO FIX ??? MAYBE FLIP SIGNS HERE AND ABOVE
  }

  printf("Pulses: %d\n", r_encoder->pulses);
}

void config_rotate(void *aux_data) {
  rotator *r_encoder = (rotator *) aux_data;
  gpio_interrupt_init();
  gpio_interrupt_config(r_encoder->rotate_a, GPIO_INTERRUPT_NEGATIVE_EDGE, true);
  gpio_interrupt_register_handler(r_encoder->rotate_a, handler, aux_data);
  gpio_interrupt_enable(r_encoder->rotate_a);
  interrupts_global_enable();
}

void count_pulses() {

}