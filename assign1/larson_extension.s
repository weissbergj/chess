/* File: larson.s
 * --------------
 * ***** EXTENSION *****
 */

# Initialize the PB0,...,PB7 registers as output registers
lui a0, 0x2000
li a1, 0x11111111
sw a1, 0x30(a0)


li a2, 0b0000000111      # Primary flow of 5 LEDs (starts at 3 in beginning)
li a3, 0b1111100000      # Check if the 5 LEDs reach the end (3 left at end, not five; see video)
li a4, 0b0000000111      # Check if the 5 LEDs reach the beginning (3 at beginning)
li a5, 0b0000000011      # Dims LEDs 1,5
li a6, 0b0000000001      # Dims LEDs 1,2,4,5
li a7, 0b0000011111      # Check if we have exceeded the 3 LED beginning and lit 5 LEDs (--> no more addi)


forward:
  jal t2, big_delay      # Set LEDs with dimming enabled from the big_delay function
  sw zero, 0x40(a0)      # turn off LEDs
  slli a2, a2, 1         # Shift the LEDs left one in these three lines
  slli a5, a5, 1         
  slli a6, a6, 1         

  blt a2, a7, shift_add  # When shifting a2, it shifts to 1110, although we want 1111, so we addi
  bne a2, a3, forward    # Check if a2 has reached the end; if not, loop again
  beq a2, a3, backward   # If a2 has reached the end, jump to the reverse function
  

shift_add:
  addi a2, a2, 1
  blt a5, a4, shift_add_a5 # a5 left shifts to 110; we want 111, so if a5<111, jump to shift_add_a5 to addi
  bne a2, a3, forward    # Check again if equal; if not return
  beq a2, a3, backward
 
shift_add_a5:
  addi a5, a5, 1
  bne a2, a3, forward
  beq a2, a3, backward


# This is the backward pass of the loop; it follows the same logic as the forward pass except we shifted
# in terms of 10 bits, so we do not have to addi -1 (subtract) to get three LEDs at the end

backward:
  jal t2, big_delay
  sw zero, 0x40(a0)
  srli a2, a2, 1
  srli a5, a5, 1
  srli a6, a6, 1

  bne a2, a4, backward

  j forward


# Set a delay that alternates rapidly between the dimming settings a2,a5,a6 with a bigger outer loop t set to 0x6

big_delay:
  li t1, 0x5
big_delay_loop:
  sw a2, 0x40(a0)
  jal ra, delay1
  
  sw a5, 0x40(a0)
  jal ra, delay2

  sw a6, 0x40(a0)
  jal ra, delay3
  
  addi t1, t1, -1
  bne t1, zero, big_delay_loop

  jalr zero, 0(t2)


# The below are the smaller delays for each iteration; the function jumps back to the big_delay_loop
# longer delays (delay2, delay3) are kept for times when fewer LEDs are activated (a5,a6)

delay1:
  lui t0, 0x50
delay1_loop:
  addi t0, t0, -1
  bne t0, zero, delay1_loop
  jalr zero, 0(ra)

delay2:
  lui t0, 0x150
delay2_loop:
  addi t0, t0, -1
  bne t0, zero, delay2_loop
  jalr zero, 0(ra)

delay3:
  lui t0, 0x200
delay3_loop:
  addi t0, t0, -1
  bne t0, zero, delay3_loop
  jalr zero, 0(ra)


