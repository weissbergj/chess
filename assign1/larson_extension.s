/* File: larson.s
 * --------------
 * ***** EXTENSION *****
 */

# Run make run_extension for Larson Scanner
lui a0, 0x2000           # Init PB0,...PB7 as output registers
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
  jal ra, check_bne_beq  # Check if a2 has reached the end and jump to reverse; if not, loop again
 
shift_add:
  addi a2, a2, 1
  blt a5, a4, shift_add_a5    # a5 left shifts to 110; we want 111, so if a5<111, jump to shift_add_a5 to addi
  jal ra, check_bne_beq 

shift_add_a5:
  addi a5, a5, 1
  jal ra, check_bne_beq  

check_bne_beq:
  blt a2, a3, forward    # Check if a2 has reached the end; if not, loop again
  j backward             # If a2 has reached the end, jump to the backward pass (i.e., reverse LED function)
  jalr zero, 0(ra)

backward:                # Backward pass of loop; similar to foward pass but 10 bits, so no add -1 to get 3 LEDs at end
  jal t2, big_delay
  sw zero, 0x40(a0)
  srli a2, a2, 1
  srli a5, a5, 1
  srli a6, a6, 1

  bne a2, a4, backward
  j forward

big_delay:              # Delay that rapidly alternates between dimming settings a2,a5,a6 w bigger outer loop t, 0x5
  li t1, 0x5            # Outer loop
big_delay_loop:
  sw a2, 0x40(a0)
  lui t0, 0x50          # Loads value that will run in delay function
  jal ra, delay
  
  sw a5, 0x40(a0)
  lui t0, 0x150
  jal ra, delay

  sw a6, 0x40(a0)
  lui t0, 0x200
  jal ra, delay
  
  addi t1, t1, -1
  bne t1, zero, big_delay_loop

  jalr zero, 0(t2)

delay:                  # Smaller delays for each iteration; longer times t0 when fewer LEDs active (a5,a6) to dim
  add t0, t0, -1
  bne t0, zero, delay
  jalr zero, 0(ra)
