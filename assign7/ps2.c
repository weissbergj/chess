/* File: ps2_assign7.c
 * -------------------
 * ***** Pasted assign5/ps2.c and added interrupt handling / optimized rather than reduced LOC *****
 */
#include "gpio.h"
#include "gpio_extra.h"
#include "malloc.h"
#include "ps2.h"
#include "gpio_interrupt.h"
#include "ringbuffer.h"

static void ps2_clock_edge_handler(void *device_ptr);

struct ps2_device {
    gpio_id_t clock_gpio;
    gpio_id_t data_gpio;
    rb_t *scancode_buffer;
    uint8_t scancode_in_progress;
    uint8_t bits_received;
    uint8_t parity_count;
};

ps2_device_t *ps2_new(gpio_id_t clock_gpio, gpio_id_t data_gpio) {
    ps2_device_t *dev = malloc(sizeof(*dev));
    
    dev->clock_gpio = clock_gpio;
    dev->data_gpio = data_gpio;
    
    gpio_set_input(clock_gpio);
    gpio_set_pullup(clock_gpio);
    gpio_set_input(data_gpio);
    gpio_set_pullup(data_gpio);
    
    dev->scancode_buffer = rb_new();
    dev->bits_received = dev->scancode_in_progress = dev->parity_count = 0;
    
    gpio_interrupt_init();
    gpio_interrupt_config(clock_gpio, GPIO_INTERRUPT_NEGATIVE_EDGE, false);
    gpio_interrupt_register_handler(clock_gpio, ps2_clock_edge_handler, dev);
    gpio_interrupt_enable(clock_gpio);
    
    return dev;
}

static void ps2_clock_edge_handler(void *device_ptr) {
    ps2_device_t *dev = (ps2_device_t *)device_ptr;
    int bit = gpio_read(dev->data_gpio);
    gpio_interrupt_clear(dev->clock_gpio);
    
    switch(dev->bits_received) {
        case 0:
            dev->scancode_in_progress = 0;
            dev->parity_count = 0;
            break;
        case 1 ... 8:
            dev->scancode_in_progress |= (bit << (dev->bits_received - 1));
            dev->parity_count ^= bit;
            break;
        case 9:
            dev->parity_count ^= bit;
            break;
        case 10:
            rb_enqueue(dev->scancode_buffer, dev->scancode_in_progress);
            dev->bits_received = -1;
            break;
    }
    dev->bits_received++;
}

uint8_t ps2_read(ps2_device_t *dev) {
    int code;
    while (!rb_dequeue(dev->scancode_buffer, &code)) { }
    return code;
}