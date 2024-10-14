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
uint8_t digit_array[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F}; // array for num 0-9 in 0b... 0 is A-F; 1 is B-C

void init_gpio(void) {
    for (int i = 0; i < 7; i++) gpio_set_output(segment[i]); // configure segments
    for (int i = 0; i < 4; i++) gpio_set_output(digit[i]);   // configure digits
    gpio_set_input(button);                                  // configure button
}

void display_digit(int num) {
    if (num < 0 || num > 9) return;    
        for (int i = 0; i < 7; i ++)
            gpio_write(segment[i], (digit_array[num] & 1 << i));  // this could go on prev line but kept for readability :(
}

void display_refresh_delay(int num, int delay) {
    if (num < 0 || num > 9999) return;
    
    for (uint64_t end_time = timer_get_ticks() + delay * 1000 * TICKS_PER_USEC;
        timer_get_ticks() < end_time && gpio_read(button);) {  // implement delay and check button
        for (int j = 3, display_num = num; j >= 0; j--, display_num /= 10) { // loop through four displays 0-3
            gpio_write(digit[j], 1);
            display_digit(display_num % 10);
            timer_delay_us(1000);                              // short delay per display 0-3
            gpio_write(digit[j], 0);
        }
    }
}

void countdown(int time) {
    if (time < 0 || time > 9959 || (time % 100) >= 60) return;  // check for edge cases/null time
    while (time >= 0) {
        display_refresh_delay(time, 1000);
        time = (time % 100) ? time - 1 : time - 41;  // same as min-- && sec = 59 cuz (-100 + 59) = -41
    }
}

void main(void) {
    init_gpio();
    countdown(107);
} // My creative output was refining my code -> efficient as possible :) learned ternary,unions,etc.; took more time than flashy end display