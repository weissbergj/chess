/* File: clock.c
 * -------------
 * ***** TODO: add your file header comment here *****
 */
#include "gpio.h"
#include "timer.h"
#include <stdint.h>

gpio_id_t segment[7] = {GPIO_PD17, GPIO_PB6, GPIO_PB12, GPIO_PB11, GPIO_PB10, GPIO_PE17, GPIO_PD11};
gpio_id_t digit[4] = {GPIO_PB4, GPIO_PB3, GPIO_PB2, GPIO_PC0};
gpio_id_t button = GPIO_PG13;

uint8_t digit_array[10] = {  // initialize array for each config of numbers 0-9 on clock; 0 is A-F, not G; 1 is B-C
    0b00111111,  // 0
    0b00000110,  // 1
    0b01011011,  // 2
    0b01001111,  // 3
    0b01100110,  // 4
    0b01101101,  // 5
    0b01111101,  // 6
    0b00000111,  // 7
    0b01111111,  // 8
    0b01101111   // 9
};

void init_gpio(void) {
        for (int i = 0; i < 7; i++) {  // configure segments
        gpio_set_output(segment[i]);
    }
    for (int i = 0; i < 4; i++) {  // configure digits
        gpio_set_output(digit[i]);
    }
    gpio_set_input(button); // configure button
}

void display_digit(int num) {
    if (num < 0 || num > 9) {
        return;
    }

    uint8_t display = digit_array[num];
    
    for (int i = 0; i < 7; i ++) {
        gpio_write(segment[i], (display & 1 << i));
    }
}

void display_write_refresh(int num) {
    if (num < 0 || num > 9999) return;
    
    for (int j = 3; j >= 0; j--) {
        gpio_write(digit[j], 1);
        display_digit(num % 10);
        timer_delay_us(1000);
        gpio_write(digit[j], 0);
        num /= 10;
    }
}

void check_digits(void) {
    while (1) {
        for (int i = 0; i <= 9999; i ++) {
            display_write_refresh(i);
            if (gpio_read(button) == 0) return;
        }
    }
}

void countdown(int time) {
    if (time < 0 || time > 9959) return;
    
    int min = time / 100;
    int sec = time % 100;

    if (sec >= 60) return;

    while (min > 0 || sec > 0) {
        int display_value = (min * 100) + sec;

        uint64_t start_time = timer_get_ticks();
        uint64_t end_time = start_time + 1000000 * TICKS_PER_USEC;

        while (timer_get_ticks() < end_time) {
            display_write_refresh(display_value);
            if (gpio_read(button) == 0) return;
        }

        if (sec == 0) {
            min--;
            sec = 59;
        } else {
            sec--;
        }
    }
    display_write_refresh(0);
}

void main(void) {
    init_gpio();
    countdown(9959);
}