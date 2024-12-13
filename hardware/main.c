// // #include "gpio.h"
// // #include "printf.h"
// // #include "timer.h"
// // #include "rotaryencoder.h"
// // #include "mymodule.h"
// // #include "servo.h"
// // #include "ik.h"
// // #include "electromagnet.h"
// #include "math_float.h"
// #include "malloc.h"
// #include "gpio_interrupt.h"

// // Constants for inverse kinematics
// #define L1 16
// #define L2 16

// // Servo motor definitions
// #define CLOCKWISE 1000
// #define COUNTER_CLOCKWISE 2000
// #define NEUTRAL 1500
// #define PERIOD 20000
// #define FULL_ROTATION_CENTRE 100000000
// #define FULL_ROTATION_OUTER 120000000
// #define TICKS_PER_SEC 24000000

// typedef struct {
//     gpio_id_t pin;
//     enum { CENTRE = 0, OUTER } type;
//     unsigned long rotation;
// } servo_motor;

// typedef struct rotator {
//     gpio_id_t rotate_a;
//     gpio_id_t rotate_b;
//     int pulses;
// } rotator;

// // Function prototypes
// void servo_init(servo_motor servo);
// void rotate(float angle, servo_motor servo, rotator* encoder);
// int calculate_angles(float x, float y, float *theta1, float *theta2);
// rotator* rotary_init(gpio_id_t rotate_a, gpio_id_t rotate_b);
// void handler(void *aux_data);
// void config_rotate(void *aux_data);
// void activate_pullup(gpio_id_t id);
// void electromagnet_init(gpio_id_t pin);
// void set_electromagnet(const int setting);
// void say_hello(const char *name);

// // Function implementations
// void servo_init(servo_motor servo) {
//     gpio_set_output(servo.pin);
//     return;
// }

// void rotate(float angle, servo_motor servo, rotator* encoder) {
//     int direction;

//     if (angle < 0) {
//         direction = COUNTER_CLOCKWISE;
//         angle *= (-1);
//     } else {
//         direction = CLOCKWISE;
//     }

//     unsigned int total_pulses = angle / 360 * TOTAL_PULSES;
//     printf("%d, ticks \n", total_pulses);

//     while (encoder->pulses <= total_pulses) {
//         gpio_write(servo.pin, 1);
//         timer_delay_us(direction);
//         gpio_write(servo.pin, 0);
//         timer_delay_us(PERIOD - direction);
//     }
// }

// int calculate_angles(float x, float y, float *theta1, float *theta2) {
//     float r = sqrt(x * x + y * y);
//     float cos_theta2 = (r * r - L1 * L1 - L2 * L2) / (2 * L1 * L2);
//     float sin_theta2 = sqrt(1 - cos_theta2 * cos_theta2);

//     *theta2 = atan2(sin_theta2, cos_theta2) * (180 / PI);
//     float phi = atan2(y, x);
//     float beta = atan2(L2 * sin_theta2, L1 + L2 * cos_theta2);
//     *theta1 = (phi - beta) * (180 / PI);

//     return 0;
// }

// rotator* rotary_init(gpio_id_t rotate_a, gpio_id_t rotate_b) {
//     rotator *r_encoder = malloc(sizeof(*r_encoder));
//     r_encoder->rotate_a = rotate_a;
//     r_encoder->rotate_b = rotate_b;
//     r_encoder->pulses = 0;
//     gpio_set_input(rotate_b);
//     gpio_set_input(rotate_a);
//     activate_pullup(rotate_a);
//     activate_pullup(rotate_b);
//     config_rotate((void *) r_encoder);
//     return r_encoder;
// }

// void handler(void *aux_data) {
//     rotator *r_encoder = (rotator *) aux_data;
//     int rotate_b_state = gpio_read(r_encoder->rotate_b);
//     gpio_interrupt_clear(r_encoder->rotate_a);

//     if (rotate_b_state) {
//         r_encoder->pulses++;
//     } else {
//         r_encoder->pulses--;
//     }

//     printf("Pulses: %d\n", r_encoder->pulses);
// }

// void config_rotate(void *aux_data) {
//     rotator *r_encoder = (rotator *) aux_data;
//     gpio_interrupt_init();
//     gpio_interrupt_config(r_encoder->rotate_a, GPIO_INTERRUPT_NEGATIVE_EDGE, true);
//     gpio_interrupt_register_handler(r_encoder->rotate_a, handler, aux_data);
//     gpio_interrupt_enable(r_encoder->rotate_a);
//     interrupts_global_enable();
// }

// void activate_pullup(gpio_id_t id) {
//     // Implementation for activating pull-up resistors
// }

// void electromagnet_init(gpio_id_t pin) {
//     gpio_set_output(pin);
//     gpio_write(pin, 0);
// }

// void set_electromagnet(const int setting) {
//     gpio_write(ELECTROMAGNET_PIN, setting);
// }

// void say_hello(const char *name) {
//     printf("Hello, %s!\n", name);
// }

// void main(void) {
//     interrupts_init();
//     uart_init();
//     rotator* encoder = rotary_init(GPIO_PB1, GPIO_PD14);
//     servo_motor servo_centre;
//     servo_motor servo_outer;

//     servo_centre.pin = GPIO_PB3;
//     servo_centre.type = CENTRE;
//     servo_centre.rotation = FULL_ROTATION_CENTRE;

//     servo_outer.pin = GPIO_PB4;
//     servo_outer.type = OUTER;
//     servo_outer.rotation = FULL_ROTATION_OUTER;

//     servo_init(servo_outer);
//     servo_init(servo_centre);

//     interrupts_global_enable();

//     float theta1 = 0.0;
//     float theta2 = 0.0;
//     calculate_angles(0.5, 0.5, &theta1, &theta2);
//     int angle = theta1;
//     int angle2 = theta2;
//     printf("%d, theta1, %d theta2 ", angle, angle2);

//     rotate(180, servo_outer, encoder);

//     while(true) {}

//     say_hello("CS107e");
// }