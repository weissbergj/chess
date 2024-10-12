/* File: gpio.c
 * ------------
 * ***** TODO: add your file header comment here *****
 */
#include "gpio.h"
#include <stddef.h>
#include <stdint.h> 
// I would rather do stding.h for uint32_t than just unsinged int; if this is not allowed, as a good mental exercise imagine the values are unsigned int lol

// I originally defined the get_group_and_index, gpio_id_is_valid, and "get_cfg" functions myself in another file not knowing they existed and were "free." Then I realized I had to edit gpio.c, not gpio.h and they were already defined in this file. I wouldn't have really understood bitwise operators or bool functions had I not done that. It is probably reasonable to except us to define every single function in this document and header, even the enumerate...They are cool things to know.




enum { GROUP_B = 0, GROUP_C, GROUP_D, GROUP_E, GROUP_F, GROUP_G };

typedef struct  {
    unsigned int group;
    unsigned int pin_index;
} gpio_pin_t;


static gpio_pin_t get_group_and_index(gpio_id_t gpio);


/* Define a struct for gpio with spacing of 32 bits for each of cfg0/1/2/3, dat, etc.
 */
typedef struct {
	volatile uint32_t cfg[4];
	volatile uint32_t dat;
	volatile uint32_t drv[4];
	volatile uint32_t pull[2];
	volatile uint32_t padding;
} gpio_group_t;

#define GPIO_BASE_ADDRESS 0x02000000

static volatile gpio_group_t *gpio_group[6] = {
	(volatile gpio_group_t *)(GPIO_BASE_ADDRESS + 0x30 * 0),  // GROUP_B
    	(volatile gpio_group_t *)(GPIO_BASE_ADDRESS + 0x30 * 1),  // GROUP_C
    	(volatile gpio_group_t *)(GPIO_BASE_ADDRESS + 0x30 * 2),  // GROUP_D
    	(volatile gpio_group_t *)(GPIO_BASE_ADDRESS + 0x30 * 3),  // GROUP_E
    	(volatile gpio_group_t *)(GPIO_BASE_ADDRESS + 0x30 * 4),  // GROUP_F
    	(volatile gpio_group_t *)(GPIO_BASE_ADDRESS + 0x30 * 5)   // GROUP_G
};


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
	if (group >= 6 || group < 0) {
		return NULL;
	} else {
		return &(gpio_group[group]->cfg[0]); // Notice how we could replace 0 with nums 1,2,3
						    // Alternatively, calculate the offset
	    }
}

// This helper function is suggested to return the address of
// the data register for a gpio group. Refer to the D1 user manual
// to learn the address of the data register for each group.
static volatile unsigned int *get_data_reg(unsigned int group) {
	if (group >= 6 || group < 0) {
	    return NULL;
    	} else {
		return &(gpio_group[group]->dat);
    	}
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

bool function_is_valid(unsigned int function) {
	return ((function >= 0 && function <= 8) || function == 14 || function == 15);
}

void gpio_set_function(gpio_id_t pin, unsigned int function) {
	if (gpio_id_is_valid(pin) && function_is_valid(function)) {
		
		gpio_pin_t gp = get_group_and_index(pin);
		volatile unsigned int *cfg_register = &(gpio_group[gp.group]->cfg[gp.pin_index / 8]); // See above for detail
		unsigned int shifter = (gp.pin_index % 8) * 4; // Determine binary shift
		*cfg_register &= ~(0xF << shifter);            // Create a mask, negate it, and merge with register

		*cfg_register |= (function << shifter); // OR the shifted function into the empty space
						
	} else if (gpio_id_is_valid(pin) && (!function_is_valid(function))) {
		//return -2;
	} else if ((!gpio_id_is_valid(pin)) && function_is_valid(function)) {
		//return GPIO_INVALID_REQUEST;
	} else {
		//return -3;
	}
}

unsigned int gpio_get_function(gpio_id_t pin) {
	if (gpio_id_is_valid(pin)) {

		gpio_pin_t gp = get_group_and_index(pin);
		volatile unsigned int *cfg_register = &(gpio_group[gp.group]->cfg[gp.pin_index / 8]);

		unsigned int shifter = (gp.pin_index % 8) * 4;
		
		unsigned int cfg_mask = 0xF << shifter;     // shifts 1111 to the position we want to extract
		cfg_mask = *cfg_register & cfg_mask; // extracts the four bits in the 1111 position in a mutable cfg
		cfg_mask = cfg_mask >> shifter;      // shifts bits to count properly e.g., dont want 11110000 = 240
		
		return cfg_mask;                     // returns the decimal of the remaining binary
	}
	return 0; // Pin is invalid ERROR
}


void gpio_write(gpio_id_t pin, int value) {
    /***** TODO: Your code goes here *****/
}

int gpio_read(gpio_id_t pin) {
    /***** TODO: Your code goes here *****/
    return 0;
}
