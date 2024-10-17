/* File: clock.c
 * -------------
 * ***** This file controls the clock functionality: delays, writing to display, countdown; it also contains the extension *****
 */
#include "gpio.h"
#include "timer.h"
#include <stdint.h>

gpio_id_t segment[7] = {GPIO_PD17, GPIO_PB6, GPIO_PB12, GPIO_PB11, GPIO_PB10, GPIO_PE17, GPIO_PD11};
gpio_id_t digit[4] = {GPIO_PB4, GPIO_PB3, GPIO_PB2, GPIO_PC0};
gpio_id_t rotary[3] = {GPIO_PB8, GPIO_PB9, GPIO_PB5}; // PB8 = A, PB9 = B, PB5 = D for rotary
gpio_id_t button = GPIO_PG13;
gpio_id_t buzzer = GPIO_PB1;
uint8_t digit_array[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F}; // array for num 0-9 in 0b... 0 is A-F; 1 is B-C

void init_gpio(void) {
    for (int i = 0; i < 7; i++) gpio_set_output(segment[i]); // configure segments
    for (int i = 0; i < 4; i++) gpio_set_output(digit[i]);   // configure digits
    for (int i = 0; i < 3; i++) gpio_set_input(rotary[i]);   // configure rotary
    gpio_set_input(button);                                  // configure button

}

void display_digit(int num) {
    if (num < 0 || num > 9) return;    
        for (int i = 0; i < 7; i ++)
            gpio_write(segment[i], (digit_array[num] & 1 << i));  // this could go on prev line but kept for readability :(
}

void display_refresh(int num) {
    if (num < 0 || num > 9999) return;
    for (int j = 3, display_num = num; j >= 0; j--, display_num /= 10) { // loop through four displays 0-3
        gpio_write(digit[j], 1);
        display_digit(display_num % 10);
        timer_delay_us(1000);                              // short delay per display 0-3
        gpio_write(digit[j], 0);
    }
}

void display_refresh_delay(int num, int delay) {
    if (num < 0 || num > 9999) return;
    for (uint64_t end_time = timer_get_ticks() + delay * TICKS_PER_USEC; // prev times another 1000
        timer_get_ticks() < end_time && gpio_read(button);) {  // implement delay and check button
        display_refresh(num);
    }
}

void countdown(int time) {
    if (time < 0 || time > 9959 || (time % 100) >= 60) return;  // check for edge cases/null time
    while (time >= 0) {
        display_refresh_delay(time, 1000);
        time = (time % 100) ? time - 1 : time - 41;  // same as min-- && sec = 59 cuz (-100 + 59) = -41
    }
}

void countdown_with_alarm(int time) {
    if (time < 0 || time > 9959 || (time % 100) >= 60) return;  // check for edge cases/null time
    while (1) {
        if (time >= 0) {
        display_refresh_delay(time, 1000 * 1000);
        time = (time % 100) ? time - 1 : time - 41;  // same as min-- && sec = 59 cuz (-100 + 59) = -41
        } else {
            gpio_set_output(buzzer);            // configure buzzer
            gpio_write(buzzer, 1);
            timer_delay_ms(10000);
            if (gpio_read(button) == 0) return;
        }
    }
}

void rotary_func(void) {

    // Quick flash to indicate start
    for (int i = 0; i < 4; i++) {
        gpio_write(digit[i], 1);
    }
    for (int i = 0; i < 7; i++) {
        gpio_write(segment[i], 1);
    }
    timer_delay_ms(500);  // Light up for 500ms
    for (int i = 0; i < 4; i++) {
        gpio_write(digit[i], 0);
    }
    for (int i = 0; i < 7; i++) {
        gpio_write(segment[i], 0);
    }
    timer_delay_ms(250);  // Stay off for 250ms

    int count = 0;
    uint64_t last_refresh = 0;
    int prevA = gpio_read(rotary[0]);
    int prevB = gpio_read(rotary[1]);

    while(1) {        
        int A = gpio_read(rotary[0]);
        int B = gpio_read(rotary[1]);

        if (A != prevA || B != prevB) {  //check if position changed; this is left at 2 changes for faster scrolling
            if (A != prevA) {
                if (B != A) count--;
                else count++;
            }

            if (count > 9959) count = 0;   // checks for range
            else if (count < 0) count = 9959;
            else if (count % 100 == 60) count += 40;
            else if (count % 100 == 99) count -= 40;

            prevA = A;
            prevB = B;
        }

        if (timer_get_ticks() - last_refresh >= 20000) {  // refresh display at interval
            display_refresh(count);
            last_refresh = timer_get_ticks();
        }
        
        if (gpio_read(rotary[2]) == 0) {  // Check for button press
            timer_delay_us(5000); // debounce
            if (gpio_read(rotary[2]) == 0) {
                countdown_with_alarm(count);
            }
        }
        timer_delay_us(100);
    }
}

void main(void) {
    init_gpio();
    rotary_func();
    // countdown(107);
}