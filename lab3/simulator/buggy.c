/* File: buggy.c
 * -------------
 * This buggy program uses a local variable that is not properly
 * initialized. The contents will be garbage/unpredictable.
 * Run this program on the Pi, and then again in gdb under
 * simulation.  Are the garbage contents the same?  Do you get
 * the same results on subsequent runs? 
 */
#include "printf.h"
#include "uart.h"

void stack_array(void) {
    int array[100];         // not initialized

    printf("array[] = {");
    for (int i = 0; i < 4; i++) {
        printf(" 0x%x ", array[i]);
        array[i]++;     // increment starting from garbage
    }
    printf("}\n");
}

void main(void) {
    uart_init();

    printf("1st call stack_array (no initialization): ");
    stack_array();
    printf("2nd call stack_array (contents leftover): ");
    stack_array();
}
