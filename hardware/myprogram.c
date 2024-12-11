#include "uart.h"
#include "mymodule.h"
#include "servo.h"
#include "gpio.h"
#include "ik.h"
#include "printf.h"
#include "electromagnet.h"
#include "timer.h"
#include "interrupts.h"
#include "rotaryencoder.h"

void main(void) {
    interrupts_init();
    uart_init();
    rotator* encoder = rotary_init(GPIO_PB1, GPIO_PD14);
    servo_motor servo_centre;
    servo_motor servo_outer;

    servo_centre.pin = GPIO_PB3;
    servo_centre.type = CENTRE;
    servo_centre.rotation = FULL_ROTATION_CENTRE;

    servo_outer.pin = GPIO_PB4;
    servo_outer.type = OUTER;
    servo_outer.rotation = FULL_ROTATION_OUTER;

    servo_init(servo_outer);
    servo_init(servo_centre);

    interrupts_global_enable();


    float theta1 = 0.0;
    float theta2 = 0.0;
    calculate_angles(0.5, 0.5, &theta1, &theta2);
    int angle = theta1;
    int angle2 = theta2;
    printf("%d, theta1, %d theta2 ", angle, angle2);

    rotate(180, servo_centre, encoder);
    rotate(180, servo_outer, encoder);


    while(true) {}


    say_hello("CS107e");
}