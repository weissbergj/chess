/* File: gpio.c
 * ------------
 * ***** TODO: add your file header comment here *****
 */
#include "gpio.h"
#include <stddef.h>

enum { GROUP_B = 0, GROUP_C, GROUP_D, GROUP_E, GROUP_F, GROUP_G };

typedef struct  {
    unsigned int group;
    unsigned int pin_index;
} gpio_pin_t;

// The gpio_id_t enumeration assigns a symbolic constant for each
// in such a way to use a single hex constant. The more significant
// hex digit identifies the group and lower 2 hex digits are pin index:
//       constant 0xNnn  N = which group,  nn = pin index within group
//
// This helper function extracts the group and pin index from a gpio_id_t
// e.g. GPIO_PB4 belongs to GROUP_B and has pin_index 4
static gpio_pin_t get_group_and_index(gpio_id_t gpio) {
    gpio_pin_t gp;
    gp.group = gpio >> 8;
    gp.pin_index = gpio & 0xff; // lower 2 hex digits
    return gp;
}

// The gpio groups are differently sized, e.g. B has 13 pins, C only 8.
// This helper function confirms that a gpio_id_t is valid (group
// and pin index are valid)
bool gpio_id_is_valid(gpio_id_t pin) {
    gpio_pin_t gp = get_group_and_index(pin);
    switch (gp.group) {
        case GROUP_B: return (gp.pin_index <= GPIO_PB_LAST_INDEX);
        case GROUP_C: return (gp.pin_index <= GPIO_PC_LAST_INDEX);
        case GROUP_D: return (gp.pin_index <= GPIO_PD_LAST_INDEX);
        case GROUP_E: return (gp.pin_index <= GPIO_PE_LAST_INDEX);
        case GROUP_F: return (gp.pin_index <= GPIO_PF_LAST_INDEX);
        case GROUP_G: return (gp.pin_index <= GPIO_PG_LAST_INDEX);
        default:      return false;
    }
}

// This helper function is suggested to return the address of
// the config0 register for a gpio group, i.e. get_cfg0_reg(GROUP_B)
// Refer to the D1 user manual to learn the address the config0 register
// for each group. Be sure to note how the address of the config1 and
// config2 register can be computed as relative offset from config0.
static volatile unsigned int *get_cfg0_reg(unsigned int group) {
    /***** TODO: Your code goes here *****/
    return NULL;
}

// This helper function is suggested to return the address of
// the data register for a gpio group. Refer to the D1 user manual
// to learn the address of the data register for each group.
static volatile unsigned int *get_data_reg(unsigned int group) {
    /***** TODO: Your code goes here *****/
    return NULL;
}

void gpio_init(void) {
    // no initialization required for this peripheral
}

void gpio_set_input(gpio_id_t pin) {
    gpio_set_function(pin, GPIO_FN_INPUT);
}

void gpio_set_output(gpio_id_t pin) {
    gpio_set_function(pin, GPIO_FN_OUTPUT);
}

void gpio_set_function(gpio_id_t pin, unsigned int function) {
   /***** TODO: Your code goes here *****/
}

unsigned int gpio_get_function(gpio_id_t pin) {
    /***** TODO: Your code goes here *****/
    return 0;
}

void gpio_write(gpio_id_t pin, int value) {
    /***** TODO: Your code goes here *****/
}

int gpio_read(gpio_id_t pin) {
    /***** TODO: Your code goes here *****/
    return 0;
}
