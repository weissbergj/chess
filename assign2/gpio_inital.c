/* File: gpio.c
 * ------------
 * ***** TODO: add your file header comment here *****
 */
#include "gpio.h"
#include <stddef.h>
#include <stdint.h> // I would rather do this for uint32_t than just unsinged int

enum { GROUP_B = 0, GROUP_C, GROUP_D, GROUP_E, GROUP_F, GROUP_G };

typedef struct  {
    unsigned int group;
    unsigned int pin_index;
} gpio_pin_t;


/* Define a struct for gpio with spacing of 32 bits for each of cfg0/1/2/3, dat, etc.
 */
typedef struct {
	volatile uint32_t cfg[4];
	volatile uint32_t dat;
	volatile uint32_t drv[4];
	volatile uint32_t pull[2];
	volatile uint32_t padding;
} gpio_struct;


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



// I initially did the following in another file because I didn't see you gave us so many free functions :)
// Free is nice but makes it harder to understand what is happening; I liked how we had 0 info for larson
// 
//bool gpio_id_is_valid(gpio_id_t pin) {
//	return (pin >= GPIO_PB0 && pin <= GPIO_PB_LAST_INDEX) || 
//		(pin >= GPIO_PC0 && pin <= GPIO_PC_LAST_INDEX) ||
//		(pin >= GPIO_PD0 && pin <= GPIO_PD_LAST_INDEX) ||
//		(pin >= GPIO_PE0 && pin <= GPIO_PE_LAST_INDEX) ||
//		(pin >= GPIO_PF0 && pin <= GPIO_PF_LAST_INDEX) ||
//		(pin >= GPIO_PG0 && pin <= GPIO_PG_LAST_INDEX);
//}
//void gpio_set_function(gpio_id_t pin, unsigned int function) {
//	if (gpio_id_is_valid(pin) && function_is_valid(function)) {
//		unsigned int *cfg_register = which_gpio_register(pin);
//
//		unsigned int pin_index = pin & 0xFF;    // Extract lower digits nn of Nnn
//		unsigned int shifter = pin_index * 4;   // Determine binary shift
//		*cfg_register &= ~(0xF << shifter);     // create a mask, negate it, and merge with register
//
//		*cfg_register |= (function << shifter); // OR the shifted function into the empty space
//	} else if (gpio_id_is_valid(pin) && (!function_is_valid(function))) {
//		return -2;
//	} else if ((!gpio_id_is_valid(pin)) && function_is_valid(function)) {
//		return GPIO_INVALID_REQUEST;
//	} else {
//		return -3;
//	}
//}




// The gpio groups are differently sized, e.g. B has 13 pins, C only 8.
// This helper function confirms that a gpio_id_t is valid (group
// and pin index are valid)
//bool gpio_id_is_valid(gpio_id_t pin) {
//    gpio_pin_t gp = get_group_and_index(pin);
//    switch (gp.group) {
//        case GROUP_B: return (gp.pin_index <= GPIO_PB_LAST_INDEX);
//        case GROUP_C: return (gp.pin_index <= GPIO_PC_LAST_INDEX);
//        case GROUP_D: return (gp.pin_index <= GPIO_PD_LAST_INDEX);
//        case GROUP_E: return (gp.pin_index <= GPIO_PE_LAST_INDEX);
//        case GROUP_F: return (gp.pin_index <= GPIO_PF_LAST_INDEX);
//        case GROUP_G: return (gp.pin_index <= GPIO_PG_LAST_INDEX);
//        default:      return false;
//    }
//}

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





bool gpio_id_is_valid(gpio_id_t pin) {
	return (pin >= GPIO_PB0 && pin <= GPIO_PB_LAST_INDEX) || 
		(pin >= GPIO_PC0 && pin <= GPIO_PC_LAST_INDEX) ||
		(pin >= GPIO_PD0 && pin <= GPIO_PD_LAST_INDEX) ||
		(pin >= GPIO_PE0 && pin <= GPIO_PE_LAST_INDEX) ||
		(pin >= GPIO_PF0 && pin <= GPIO_PF_LAST_INDEX) ||
		(pin >= GPIO_PG0 && pin <= GPIO_PG_LAST_INDEX);
}


bool function_is_valid(unsigned int function) {
	return ((function >= 0 && function <= 8) || function == 14 || function == 15);
}

unsigned int *which_gpio_register(gpio_id_t pin) {
	unsigned int *GPIO_base_address = (unsigned int*)0x02000000;
	unsigned int *cfg_register;
	
	if (pin >= GPIO_PB0 && pin <= GPIO_PB7) {
    		cfg_register = (unsigned int *)((char *)GPIO_base_address + 0x0030);  // PB_CFG0
	} else if (pin >= GPIO_PB8 && pin <= GPIO_PB_LAST_INDEX) {
    		cfg_register = (unsigned int *)((char *)GPIO_base_address + 0x0034);  // PB_CFG1
	} else if (pin >= GPIO_PC0 && pin <= GPIO_PC7) {
    		cfg_register = (unsigned int *)((char *)GPIO_base_address + 0x0060);  // PC_CFG0
	} else if (pin >= GPIO_PD0 && pin <= GPIO_PD7) {
    		cfg_register = (unsigned int *)((char *)GPIO_base_address + 0x0090);  // PD_CFG0
	} else if (pin >= GPIO_PD8 && pin <= GPIO_PD15) {
    		cfg_register = (unsigned int *)((char *)GPIO_base_address + 0x0094);  // PD_CFG1
	} else if (pin >= GPIO_PD16 && pin <= GPIO_PD_LAST_INDEX) {
    		cfg_register = (unsigned int *)((char *)GPIO_base_address + 0x0098);  // PD_CFG2
	} else if (pin >= GPIO_PE0 && pin <= GPIO_PE7) {
    		cfg_register = (unsigned int *)((char *)GPIO_base_address + 0x00C0);  // PE_CFG0
	} else if (pin >= GPIO_PE8 && pin <= GPIO_PE_LAST_INDEX) {
    		cfg_register = (unsigned int *)((char *)GPIO_base_address + 0x00C4);  // PE_CFG1
	} else if (pin >= GPIO_PF0 && pin <= GPIO_PF_LAST_INDEX) {
    		cfg_register = (unsigned int *)((char *)GPIO_base_address + 0x00F0);  // PF_CFG0
	} else if (pin >= GPIO_PG0 && pin <= GPIO_PG7) {
    		cfg_register = (unsigned int *)((char *)GPIO_base_address + 0x0120);  // PG_CFG0
	} else if (pin >= GPIO_PG8 && pin <= GPIO_PG15) {
    		cfg_register = (unsigned int *)((char *)GPIO_base_address + 0x0124);  // PG_CFG1
	} else if (pin >= GPIO_PG16 && pin <= GPIO_PG_LAST_INDEX) {
		cfg_register = (unsigned int *)((char *)GPIO_base_address + 0x0128);  // PG_CFG2
	} else {
		return NULL;	// CONSIDER AN ERROR HERE
	}
	return cfg_register;
}

void gpio_set_function(gpio_id_t pin, unsigned int function) {
	if (gpio_id_is_valid(pin) && function_is_valid(function)) {
		unsigned int *cfg_register = which_gpio_register(pin);

		unsigned int pin_index = pin & 0xFF;    // Extract lower digits nn of Nnn
		unsigned int shifter = pin_index * 4;   // Determine binary shift
		*cfg_register &= ~(0xF << shifter);     // create a mask, negate it, and merge with register

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
		unsigned int *cfg_register = which_gpio_register(pin);
		unsigned int cfg_mask;

		unsigned int pin_index = pin & 0xFF;
		unsigned int shifter = pin_index * 4;
		
		cfg_mask = 0xF << shifter;           // shifts 1111 to the position we want to extract
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
