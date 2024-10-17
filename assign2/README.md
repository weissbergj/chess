See the clock.c file that contains code to run a clock countdown. Any value up to 9959 can be initialized. Adjust the main function to countdown(your_integer_input) and then run make run. For the extension, you can adjust the countdown to any value you like via a rotary encoder. Then press the rotary button to activate. After the countdown, a buzzer will play until interrupted by another button. Adjust the main function to rotary() and run make run. I did not make a fancy output since my buzzer broke, but the difficulty in writing this code was the rotary encoder, not the final output. Imagine the final output called a larson scanner if it suits you. See the pullup activation and read functions in the gpio.c file. Other useful files include test_gpio_timer.c and gpio.h in cs107e/include.

The extension coded on Monday (important is that all code was completed Monday) was successfully demoed to Ben on Wednesday, October 16th, with a brief flash at the beginning and ran, turning on an LED:

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
