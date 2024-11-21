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
#include "timer.h"

// First, declare the struct
struct ps2_device {
    gpio_id_t clock_gpio;
    gpio_id_t data_gpio;
    rb_t *scancode_buffer;
    uint8_t scancode_in_progress;
    uint8_t bits_received;
    uint8_t parity_count;
    bool writing;
};

// Then, declare the type alias
typedef struct ps2_device ps2_device_t;

// Now declare function prototypes
static void ps2_clock_edge_handler(void *device_ptr);
static void ps2_wait_clock_high(ps2_device_t *dev);
static void ps2_wait_clock_low(ps2_device_t *dev);

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
            if (bit == 0) {
                dev->scancode_in_progress = 0;
                dev->parity_count = 0;
            } else {
                dev->bits_received = -1;
                return;
            }
            break;
        case 1 ... 8:
            dev->scancode_in_progress |= (bit << (dev->bits_received - 1));
            dev->parity_count ^= bit;
            break;
        case 9:
            if (bit != (dev->parity_count & 1)) {
                dev->bits_received = -1;
                return;
            }
            break;
        case 10:
            if (bit != 1) {
                dev->bits_received = -1;
                return;
            }
            rb_enqueue(dev->scancode_buffer, dev->scancode_in_progress);
            break;
    }
    dev->bits_received++;
}

uint8_t ps2_read(ps2_device_t *dev) {
    int code;
    while (!rb_dequeue(dev->scancode_buffer, &code)) { }
    return code;
}

bool ps2_write(ps2_device_t *dev, uint8_t scancode) {
    gpio_interrupt_disable(dev->clock_gpio);
    
    gpio_set_output(dev->clock_gpio);
    gpio_write(dev->clock_gpio, 0);
    timer_delay_us(100);
    
    gpio_set_output(dev->data_gpio);
    gpio_write(dev->data_gpio, 0);
    
    gpio_set_input(dev->clock_gpio);
    gpio_set_pullup(dev->clock_gpio);
    
    uint8_t parity = 1;
    
    for (int i = 0; i < 8; i++) {
        ps2_wait_clock_low(dev);
        gpio_write(dev->data_gpio, (scancode >> i) & 1);
        if ((scancode >> i) & 1) parity ^= 1;
        ps2_wait_clock_high(dev);
    }
    
    ps2_wait_clock_low(dev);
    gpio_write(dev->data_gpio, parity);
    ps2_wait_clock_high(dev);
    
    ps2_wait_clock_low(dev);
    gpio_write(dev->data_gpio, 1);
    ps2_wait_clock_high(dev);
    
    gpio_set_input(dev->data_gpio);
    gpio_set_pullup(dev->data_gpio);
    
    ps2_wait_clock_low(dev);
    ps2_wait_clock_high(dev);
    
    gpio_interrupt_enable(dev->clock_gpio);
    
    return true;
}

static void ps2_wait_clock_high(ps2_device_t *dev) {
    while (!gpio_read(dev->clock_gpio)) { }
}

static void ps2_wait_clock_low(ps2_device_t *dev) {
    while (gpio_read(dev->clock_gpio)) { }
}