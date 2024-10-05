// This file contains the translation from C to assembly

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void pb_output(int value) {
  unsigned int base = 0x2000;
  unsigned int a1 = 0x00000001;

  if (value >= 0 && value < 7) {
    a1 = a1 << value;
  } else if (value < 0) {
    a1 = 0;
  }
  
  printf("lui a0, 0x%04x\n", base);
  printf("li a1, 0x%04x\n", a1);
  printf("sw a1, 0x30(a0)\n"); 
}

void pb_on(int value) {
  unsigned int base = 0x2000;
  unsigned int a1 = 0b00000001;
  
  if (value >= 0 && value <= 7) {
    a1 = 1 << value;
  }
  
  printf("lui a0, 0x%04x\n", base);
  printf("li a1, %d\n", a1);
  printf("sw a1, 0x40(a0)\n)");
}


int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Usage: %s <function> <value>\n", argv[0]);
    return 1;
    }

  int value = atoi(argv[2]);

  if (strcmp(argv[1], "output") == 0) {
    pb_output(value);
  } else if (strcmp(argv[1], "on") == 0) {
    pb_on(value);
  } else {
    printf("Invalid function. Use 'output' or 'on'.\n");
    return 1;
  }

  return 0;
}




//#
//#What I need to do:
//#
//#PB_state(-8 to 0 to 0 through 7): PB0 through 7; input / output initialization
//#  0 -->
//#  lui a0, 0x2000
//#  li a1, 0x0001
//#  sw a1, 0x30(a0)
//#
//#  this is for a1: 0x0001, 0x0010, 0x0100, 0x1000,...,0x1000000; it is just a slli with no addi
//#
//#PB_input
//#
//#PB_output_range(0 through 7)
//#  0 -->
//#  lui a0, 0x2000
//#  li a1, 0x0001
//#  sw a1, 0x30(a0)
//#
//#  if 1, slli a1, a1, 1; addi a1,a1,1
//#
//#  this is for a1: 0x0001,0x0011,0x0111,...,0x11111111
//#
//#
//#PB_input_range(0 through 7)
//#  same as PB_output_range but loads 0's instead
//#
//#PB_on(0 through 7): PB0 through 7
//#  0 -->
//#  lui a0, 0x2000
//#  li a1, 0b00000001
//#  sw a1, 0x40(a0)
//#
//#PB_on_group(accepts1,01,000111, 11111111 or 0s, etc); similar to output_range except sets specific and only accepts values of int length as big as PB_output_range's input
//#
//#PB_off(0 through 7)
//#  opposite PB
//#


