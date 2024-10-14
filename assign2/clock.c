/* File: clock.c
 * -------------
 * ***** This file controls the clock functionality: delays, writing to display, countdown *****
 */
#include "gpio.h"
#include "timer.h"
#include <stdint.h>

gpio_id_t segment[7] = {GPIO_PD17, GPIO_PB6, GPIO_PB12, GPIO_PB11, GPIO_PB10, GPIO_PE17, GPIO_PD11};
gpio_id_t digit[4] = {GPIO_PB4, GPIO_PB3, GPIO_PB2, GPIO_PC0};
gpio_id_t button = GPIO_PG13;

// initialize array for each config of numbers 0-9 on clock; 0 is A-F, not G; 1 is B-C
uint8_t digit_array[10] = {0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110, 
    0b01101101, 0b01111101, 0b00000111, 0b01111111, 0b01101111};

void init_gpio(void) {
    for (int i = 0; i < 7; i++) {    // configure segments
        gpio_set_output(segment[i]);
    } for (int i = 0; i < 4; i++) {  // configure digits
        gpio_set_output(digit[i]);
    } gpio_set_input(button);        // configure button
}

void display_digit(int num) {
    if (num < 0 || num > 9) return;
    uint8_t display = digit_array[num];
    
    for (int i = 0; i < 7; i ++) {
        gpio_write(segment[i], (display & 1 << i));
    }
}

void display_refresh_delay(int num, int delay) {
    if (num < 0 || num > 9999) return;
    
    for (uint64_t end_time = timer_get_ticks() + delay * 1000 * TICKS_PER_USEC;
        timer_get_ticks() < end_time && gpio_read(button);) {  // implement delay and check button
        for (int j = 3, display_num = num; j >= 0; j--, display_num /= 10) { // loop through four displays 0-3
            gpio_write(digit[j], 1);
            display_digit(display_num % 10);
            timer_delay_us(1000);           // short delay per display 0-3
            gpio_write(digit[j], 0);
        }
    }
}

void countdown(int time) {
    if (time < 0 || time > 9959 || (time % 100) >= 60) return;  // check for edge cases/null time

    while (time > 0) {
        display_refresh_delay(time, 1000);
        time = (time % 100) ? time - 1 : time - 41;  // same as min-- && sec = 59 cuz (-100 + 59) = -41
    }
    display_refresh_delay(0, 1000);
}

void main(void) {
    init_gpio();
    countdown(107);
}
// My creative output was really refining my code to make it as efficient as possible :)
// I learned things like ternary operators and unions; this took more time than writing done or a flashy end display; I don't want to add LOC lol