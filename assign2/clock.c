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
gpio_id_t rgb_led[2] = {GPIO_PG12, GPIO_PB7};  // Yellow on PG12, Red on PB7 (G pin to ground)

void init_gpio(void) {
    for (int i = 0; i < 7; i++) gpio_set_output(segment[i]); // configure segments
    for (int i = 0; i < 4; i++) gpio_set_output(digit[i]);   // configure digits
    for (int i = 0; i < 3; i++) {
        gpio_set_input(rotary[i]);         // configure rotary
        gpio_activate_pullup(rotary[i]);    // add pull-ups for rotary
    }
    gpio_set_input(button);                // configure button
    for (int i = 0; i < 2; i++) {
        gpio_set_output(rgb_led[i]);            // set both LED pins as outputs
    }
    gpio_set_output(buzzer);                    // configure buzzer
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
            gpio_write(buzzer, 1);              // turn on buzzer
            
            // Fun color pattern until button pressed
            while (gpio_read(button)) {
                // Flash yellow (PG12)
                gpio_write(rgb_led[0], 1);
                gpio_write(rgb_led[1], 0);
                display_refresh_delay(0, 200 * 1000);

                // Flash red (PB7)
                gpio_write(rgb_led[0], 0);
                gpio_write(rgb_led[1], 1);
                display_refresh_delay(0, 200 * 1000);

                // Flash both rapidly for effect
                for (int i = 0; i < 100; i++) {
                    gpio_write(rgb_led[0], 1);
                    gpio_write(rgb_led[1], 0);
                    timer_delay_us(10);
                    gpio_write(rgb_led[0], 0);
                    gpio_write(rgb_led[1], 1);
                    timer_delay_us(10);
                }
                display_refresh_delay(0, 100 * 1000);

                // Fade effect (slower alternating)
                for (int i = 0; i < 5; i++) {
                    gpio_write(rgb_led[0], 1);
                    gpio_write(rgb_led[1], 0);
                    display_refresh_delay(0, 50 * 1000);
                    gpio_write(rgb_led[0], 0);
                    gpio_write(rgb_led[1], 1);
                    display_refresh_delay(0, 50 * 1000);
                }

                // Both on together
                gpio_write(rgb_led[0], 1);
                gpio_write(rgb_led[1], 1);
                display_refresh_delay(0, 200 * 1000);

                // Both off
                gpio_write(rgb_led[0], 0);
                gpio_write(rgb_led[1], 0);
                display_refresh_delay(0, 100 * 1000);

                // Quick strobe effect
                for (int i = 0; i < 20; i++) {
                    gpio_write(rgb_led[0], 1);
                    gpio_write(rgb_led[1], 1);
                    timer_delay_us(5000);
                    gpio_write(rgb_led[0], 0);
                    gpio_write(rgb_led[1], 0);
                    timer_delay_us(5000);
                }
                display_refresh_delay(0, 100 * 1000);
            }
            
            // Turn everything off
            gpio_write(buzzer, 0);
            gpio_write(rgb_led[0], 0);
            gpio_write(rgb_led[1], 0);
            return;
        }
    }
}

void rotary_func(void) {
    // Letter patterns for H, I, G, O
    uint8_t H = 0x76;    // 0b01110110
    uint8_t I = 0x06;    // 0b00000110
    uint8_t G = 0x7D;    // 0b01111101
    uint8_t O = 0x3F;    // 0b00111111
    
    // Display "HI"
    for (int j = 0; j < 800; j++) {  // Show for about 0.8 seconds
        // Display H on left
        gpio_write(digit[0], 1);
        for (int i = 0; i < 7; i++) {
            gpio_write(segment[i], (H & (1 << i)));
        }
        timer_delay_us(1000);
        gpio_write(digit[0], 0);
        
        // Display I on right
        gpio_write(digit[1], 1);
        for (int i = 0; i < 7; i++) {
            gpio_write(segment[i], (I & (1 << i)));
        }
        timer_delay_us(1000);
        gpio_write(digit[1], 0);
    }
    
    // Brief pause
    timer_delay_ms(200);
    
    // Display "GO"
    for (int j = 0; j < 800; j++) {  // Show for about 0.8 seconds
        // Display G on left
        gpio_write(digit[0], 1);
        for (int i = 0; i < 7; i++) {
            gpio_write(segment[i], (G & (1 << i)));
        }
        timer_delay_us(1000);
        gpio_write(digit[0], 0);
        
        // Display O on right
        gpio_write(digit[1], 1);
        for (int i = 0; i < 7; i++) {
            gpio_write(segment[i], (O & (1 << i)));
        }
        timer_delay_us(1000);
        gpio_write(digit[1], 0);
    }
    
    // Final flash
    for (int i = 0; i < 4; i++) {
        gpio_write(digit[i], 1);
    }
    for (int i = 0; i < 7; i++) {
        gpio_write(segment[i], 1);
    }
    timer_delay_ms(200);  // Quick flash
    
    // Turn everything off
    for (int i = 0; i < 4; i++) {
        gpio_write(digit[i], 0);
    }
    for (int i = 0; i < 7; i++) {
        gpio_write(segment[i], 0);
    }
    timer_delay_ms(200);

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
                return;  // Return to main after alarm finishes
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