/* File: ps2_assign5.c
 * -------------------
 * Implementation of PS/2 protocol for reading scancodes from PS/2 devices.
 */
#include "gpio.h"
#include "gpio_extra.h"
#include "malloc.h"
#include "ps2.h"

struct ps2_device {
    gpio_id_t clock;
    gpio_id_t data;
};

// Creates a new PS2 device connected to given clock and data pins,
// The gpios are configured as input and set to use internal pull-up
// (PS/2 protocol requires clock/data to be high default)
ps2_device_t *ps2_new(gpio_id_t clock_gpio, gpio_id_t data_gpio) {
    // consider why must malloc be used to allocate device
    ps2_device_t *dev = malloc(sizeof(*dev));

    dev->clock = clock_gpio;
    gpio_set_input(dev->clock);
    gpio_set_pullup(dev->clock);

    dev->data = data_gpio;
    gpio_set_input(dev->data);
    gpio_set_pullup(dev->data);
    
    return dev;
}

static int read_bit(ps2_device_t *dev) {
    while (gpio_read(dev->clock) == 0) { }  // Wait for clock high
    while (gpio_read(dev->clock) == 1) { }  // Wait for falling edge
    int bit = gpio_read(dev->data);         // Read data bit
    while (gpio_read(dev->clock) == 0) { }  // Wait for clock to return high
    return bit;
}

uint8_t ps2_read(ps2_device_t *dev) {
    while (1) {
        if (read_bit(dev) != 0) continue;  // Start bit must be 0

        uint8_t data = 0;
        int parity = 0;
        
        // Read 8 data bits, LSB first
        for (int i = 0; i < 8; i++) {
            int bit = read_bit(dev);
            data |= (bit << i);
            parity ^= bit;  // XOR for parity calculation
        }

        parity ^= read_bit(dev);           // Include parity bit
        if (!parity || read_bit(dev) != 1) continue;  // Check parity and stop bit
        
        return data;
    }
}