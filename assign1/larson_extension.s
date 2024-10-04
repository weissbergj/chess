/* File: larson.s
 * --------------
 * ***** EXTENSION *****
 */

#
## See below for the code that addressed the direction of the LEDs for the scanner with 8 LEDs :)
#

# Initialize the 0x30 register to outputs PB0 through PB7 denoted by 0x11111111
lui a0, 0x2000
li a1, 0x11111111
sw a1, 0x30(a0)  

# Initialize several values with decimals that satisfy binaries 1, 11, ..., 11111111
li a2, 1
li a3, 2
li a4, 4
li a5, 8
li a6, 16
li t1, 32
li t2, 64
li t3, 128

loop:
  sw a2, 0x40(a0)
  jal ra, delay
  sw a3, 0x40(a0)
  jal ra, delay
  sw a4, 0x40(a0)
  jal ra, delay
  sw a5, 0x40(a0)
  jal ra, delay
  sw a6, 0x40(a0)
  jal ra, delay
  sw t1, 0x40(a0)
  jal ra, delay
  sw t2, 0x40(a0)
  jal ra, delay
  sw t3, 0x40(a0)
  jal ra, delay
  sw t2, 0x40(a0)
  jal ra, delay
  sw t1, 0x40(a0)
  jal ra, delay
  sw a6, 0x40(a0)
  jal ra, delay
  sw a5, 0x40(a0)
  jal ra, delay
  sw a4, 0x40(a0)
  jal ra, delay
  sw a3, 0x40(a0)
  jal ra, delay
  j loop

delay:
  lui t0, 0xDCC
delay_loop:
  addi t0, t0,-1
  bne t0, zero, delay_loop
  sw zero, 0x40(a0)
  jalr zero, 0(ra)

