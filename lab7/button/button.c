#include "gl.h"
#include "gpio.h"
#include "gpio_extra.h"
#include "gpio_interrupt.h"
#include "interrupts.h"
#include "printf.h"
#include "ringbuffer.h"
#include "uart.h"

static const gpio_id_t BUTTON = GPIO_PB4;
static int gCount = 0;

void handle_click(void *aux_data) {
    gCount++;
    rb_t *rb = (rb_t *)aux_data;
    rb_enqueue(rb, gCount);
    gpio_interrupt_clear(BUTTON);
}

void redraw(int nclicks) {
    static int nredraw = -1;
    // count number of redraws, alternate bg color on each redraw
    color_t bg = nredraw++ % 2 ? GL_AMBER : GL_BLUE;

    gl_clear(GL_BLACK);
    char buf[100];
    snprintf(buf, sizeof(buf), "Click count = %d (redraw #%d)", nclicks, nredraw);
    gl_draw_string(0, 0, buf, GL_WHITE);
    // intentionally slow loop for educational purposes :-)
    for (int y = gl_get_char_height(); y < gl_get_height(); y++) {
        for (int x = 0; x < gl_get_width(); x++) {
            gl_draw_pixel(x, y, bg);
        }
    }
}

void config_button(void) {
    gpio_set_input(BUTTON);
    gpio_set_pullup(BUTTON);
    gpio_interrupt_init();
    gpio_interrupt_config(BUTTON, GPIO_INTERRUPT_DOUBLE_EDGE, false);
}

void main(void) {
    gpio_init();
    uart_init();
    gl_init(800, 600, GL_SINGLEBUFFER);
    
    interrupts_init();
    
    rb_t *rb = rb_new();
    config_button();
    gpio_interrupt_register_handler(BUTTON, handle_click, rb);
    gpio_interrupt_enable(BUTTON);
    interrupts_global_enable();

    redraw(0);
    while (true) {
        int count;
        if (rb_dequeue(rb, &count)) {
            redraw(count);
        }
    }
}
