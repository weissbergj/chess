#ifndef ROTARYENCODER_H
#define ROTARYENCODER_H
typedef struct rotator {
  gpio_id_t rotate_a;
  gpio_id_t rotate_b;
  int pulses;
} rotator;


rotator* rotary_init(gpio_id_t rotate_a, gpio_id_t rotate_b);
void handler(void *aux_data);
void config_rotate(void *aux_data);
void count_pulses();
#endif
