/* File: larson.s
 * --------------
 * ***** Constructing a Naive Larson Scanner *****
 */

/*
 * The code below is the blink program from Lab 1.
 * Modify the code to implement the larson scanner for Assignment 1.
 * Be sure to use GPIO pins PB0-PB3 (or PB0-PB7) for your LEDs.
 */

# EXTENSION
# PLEASE SEE larson_extension.s AND RUN make run_extension FOR THE EXTENSION.
# EXTENSION

# I went through many iterations to get to my final code.
# 1. Final code
# 2. Blinking four LEDs simultaneously
# 3. Blinking four LEDs sequentially
# 4. Larson scanner for 8 LEDs without jumps
# 5. Larson scanner with delay function separated


# FINAL CODE BELOW
# FINAL CODE BELOW
# --------------------------------SECTION 1---------------------------------------------
# THIS IS MY FINAL CODE.

# Load the upper register of the a0 so that when we add 0x30 to it, we get 0x20030
# Then initialize the config register with all binary to say we are using PB0,...,PB7 as outputs
lui a0, 0x2000
li a1, 0x11111111
sw a1, 0x30(a0)

# Initialize the PB voltages; 1 is 3.3V; 0 is 0; each binary is PB0...PB7 little-endian;
# t0 checks that we get to PB7; t1 checks that we get back to PB0
li a2, 0b00000001
li t0, 0b10000000
li t1, 0b00000001

# Load a2 into the output register, jump and link delay function with return address ra
# Turn off the output voltage; shift a2 left (PB0 --> PB1, etc.) and then turn it on;
# Check if equal to PB8; if not, continue
equal:
  sw a2, 0x40(a0)
  jal ra, delay
  sw zero, 0x40(a0)
  slli a2, a2, 1
  bne t0, a2, equal

# This is similar to equal, except we shift a2 right and check if it is equal to the PB0 state
# Then loop back up to equal to continue indefinitely
not_equal:  
  sw a2, 0x40(a0)
  jal ra, delay
  sw zero, 0x40(a0)
  srli a2, a2, 1
  bne a2, t1, not_equal

  j equal

# This is the delay function that decrements a loaded value t2 (~0.1 sec per cycle); it jumps to register ra defined in jal ra, delay
delay:
  lui t2, 0xDCC
delay_loop:
  addi t2, t2, -1
  bne t2, zero, delay_loop
  jalr zero, 0(ra)






















# --------------------------------SECTION 2---------------------------------------------
# See below for the initial code to blink four LEDs simultaneously.
# 
#     lui     a0,0x2000        # a0 holds base addr PB group = 0x2000000
#     li      a1,0x1111        # we create the binary 0001 x 4 = 0x1111
#     sw      a1,0x30(a0)      # config PB0/1/2/3 as outputs instead of inputs by storing 0x1111
# 
#     addi    a2,a2,15         # create a value a2 that will turn on the pins via binary 1111
# 
# 
#     loop:
#       xori    a2,a2,15       # invert a1 for all binary 1
#       sw      a2,0x40(a0)    # set data value of PB0/1/2/3 to a2; this tells the pins to emit 3.3V when 1 or 0V when 0
#       lui     a3,0x4500      # a3 = init countdown value
#    
#     delay:
#       addi    a3,a3,-1       # decrement a3
#       bne     a3,zero,delay  # keep counting down until a3 is zero
# 
#       j       loop           # back to top of outer loop
# 

# --------------------------------SECTION 3---------------------------------------------
# See below for my first iteration, where I blinked 4 LEDs sequentially.
#
#     lui     a0,0x2000       
#     li      a1,0x11111111       
#     sw      a1,0x30(a0)      
# 
#     li a2,0
#     li a3,0
#     li a4,0
#     li a5,0
#     li a6,0
#
#     addi    a2,a2,1        
#     addi    a3,a3,2
#     addi    a4,a4,4
#     addi    a5,a5,8
#
#     xori    a2,a2,1
#     xori    a3,a3,2
#     xori    a4,a4,4
#     xori    a5,a5,8
# 
# 
#     loop:
#       xori    a2,a2,1      
#       sw      a2,0x40(a0)   
#       lui     a6,0x4500     
# 
#       delay:
#         addi    a6,a6,-1       
#         bne     a6,zero,delay  
#       xori a2,a2,1
#       sw   a2,0x40(a0)
#      
#
#       xori a3,a3,2
#       sw   a3,0x40(a0)
#       lui  a6,0x4500
#
#       delay2:
#         addi    a6,a6,-1      
#         bne     a6,zero,delay2
#       xori a3,a3,2
#       sw   a3,0x40(a0)
#       
#
#       xori a4,a4,4
#       sw   a4,0x40(a0)
#       lui  a6,0x4500
#
#       delay3:
#         addi    a6,a6,-1       
#         bne     a6,zero,delay3
#       xori a4,a4,4
#       sw   a4,0x40(a0)
#      
#       
#       xori a5,a5,8
#       sw   a5,0x40(a0)
#       lui  a6,0x4500
#
#       delay4:
#         addi    a6,a6,-1      
#         bne     a6,zero,delay4 
#       xori a5,a5,8
#       sw   a5,0x40(a0)
#     
#       j loop


# --------------------------------SECTION 4---------------------------------------------
# See below for the code that addressed the direction of the LEDs for the scanner with 8 LEDs :)

# Initialize the  0x30 register to outputs PB 0 through 7 denoted by 0x11111111
#     lui     a0,0x2000       
#     li      a1,0x11111111      
#     sw      a1,0x30(a0) 
#
#
## Initialize several values with decimals that satisfy the binaries 1,11,...,111111111
#     li a2,1
#     li a3,2
#     li a4,4
#     li a5,8
#     li a6,0
#     li a7,16
#     li t1,32
#     li t2,64
#     li t3,128    
#
#
## Invert values of 1 to 0
#     xori    a2,a2,1
#     xori    a3,a3,2
#     xori    a4,a4,4
#     xori    a5,a5,8
#     xori    a7,a7,16
#     xori    t1,t1,32
#     xori    t2,t2,64
#     xori    t3,t3,128
#
#
# # Set up a big loop that repeats for every complete sequence PB0,...PB7,...PB0. Every PB cycle within the loop is separated by two lines. Invert the values, load them into the register to activate the LED, set a "countdown" a6 to 0xDCC (~0.1 seconds), decrement that number until branching out to the below function. Then turn off the LED by changing the values of a2 and loading them into the 0x2000040 register. Although this was a line-by-line explanation, refer to previous sections if you don't understand.
#     loop:
#       xori    a2,a2,1   
#       sw      a2,0x40(a0)   
#       lui     a6,0xDCC      
# 
#       delay:
#         addi    a6,a6,-1      
#         bne     a6,zero,delay  
#       xori a2,a2,1
#       sw   a2,0x40(a0)
#      
#
#       xori a3,a3,2
#       sw   a3,0x40(a0)
#       lui  a6,0xDCC
#
#       delay2:
#         addi    a6,a6,-1      
#         bne     a6,zero,delay2
#       xori a3,a3,2
#       sw   a3,0x40(a0)
#       
#
#       xori a4,a4,4
#       sw   a4,0x40(a0)
#       lui  a6,0xDCC
#
#       delay3:
#         addi    a6,a6,-1      
#         bne     a6,zero,delay3
#       xori a4,a4,4
#       sw   a4,0x40(a0)
#      
#       
#       xori a5,a5,8
#       sw   a5,0x40(a0)
#       lui  a6,0xDCC
#
#       delay4:
#         addi    a6,a6,-1      
#         bne     a6,zero,delay4 
#       xori a5,a5,8
#       sw   a5,0x40(a0)
#
#
#       xori a7,a7,16
#       sw   a7,0x40(a0)
#       lui  a6,0xDCC
#
#       delay5:
#         addi    a6,a6,-1      
#         bne     a6,zero,delay5 
#       xori a7,a7,16
#       sw   a7,0x40(a0)
#
#
#       xori t1,t1,32
#       sw   t1,0x40(a0)
#       lui  a6,0xDCC
#
#       delay6:
#         addi    a6,a6,-1      
#         bne     a6,zero,delay6 
#       xori t1,t1,32
#       sw   t1,0x40(a0)
#
#
#       xori t2,t2,64
#       sw   t2,0x40(a0)
#       lui  a6,0xDCC
#
#       delay7:
#         addi    a6,a6,-1      
#         bne     a6,zero,delay7
#       xori t2,t2,64
#       sw   t2,0x40(a0)
#
#
#       xori t3,t3,128
#       sw   t3,0x40(a0)
#       lui  a6,0xDCC
#
#       delay8:
#         addi    a6,a6,-1      
#         bne     a6,zero,delay8
#       xori t3,t3,128
#       sw   t3,0x40(a0)
#
#
#       xori t2,t2,64
#       sw   t2,0x40(a0)
#       lui  a6,0xDCC
#
#       delay9:
#         addi    a6,a6,-1      
#         bne     a6,zero,delay9
#       xori t2,t2,64
#       sw   t2,0x40(a0)
#
#
#       xori t1,t1,32
#       sw   t1,0x40(a0)
#       lui  a6,0xDCC
#
#       delay10:
#         addi    a6,a6,-1      
#         bne     a6,zero,delay10
#       xori t1,t1,32
#       sw   t1,0x40(a0)
#
#
#       xori    a5,a5,16
#       sw      a5,0x40(a0)
#       lui     a6,0xDCC
#
#       delay11:
#         addi    a6,a6,-1
#         bne     a6,zero,delay11
#       xori    a5,a5,16
#       sw      a5,0x40(a0)
#
#
#       xori    a4,a4,8
#       sw      a4,0x40(a0)
#       lui     a6,0xDCC
#
#       delay12:
#         addi    a6,a6,-1
#         bne     a6,zero,delay12
#       xori    a4,a4,8
#       sw      a4,0x40(a0)
#
#
#       xori    a3,a3,4
#       sw      a3,0x40(a0)
#       lui     a6,0xDCC
#
#       delay13:
#         addi    a6,a6,-1
#         bne     a6,zero,delay13
#       xori    a3,a3,4
#       sw      a3,0x40(a0)
#
#
#       xori    a2,a2,2
#       sw      a2,0x40(a0)
#       lui     a6,0xDCC
#
#       delay14:
#         addi    a6,a6,-1
#         bne     a6,zero,delay14
#       xori    a2,a2,2
#       sw      a2,0x40(a0)
#
#
#       j loop



# --------------------------------SECTION 5--------------------------------------------
## See below for the code that addressed the direction of the LEDs for the scanner with 8 LEDs :)
#
#
## Initialize the 0x30 register to outputs PB0 through PB7 denoted by 0x11111111
#lui a0, 0x2000
#li a1, 0x11111111
#sw a1, 0x30(a0)  
#
## Initialize several values with decimals that satisfy binaries 1, 11, ..., 11111111
#li a2, 1
#li a3, 2
#li a4, 4
#li a5, 8
#li a6, 16
#li t1, 32
#li t2, 64
#li t3, 128
#
#loop:
#  sw a2, 0x40(a0)
#  jal ra, delay
#  sw a3, 0x40(a0)
#  jal ra, delay
#  sw a4, 0x40(a0)
#  jal ra, delay
#  sw a5, 0x40(a0)
#  jal ra, delay
#  sw a6, 0x40(a0)
#  jal ra, delay
#  sw t1, 0x40(a0)
#  jal ra, delay
#  sw t2, 0x40(a0)
#  jal ra, delay
#  sw t3, 0x40(a0)
#  jal ra, delay
#  sw t2, 0x40(a0)
#  jal ra, delay
#  sw t1, 0x40(a0)
#  jal ra, delay
#  sw a6, 0x40(a0)
#  jal ra, delay
#  sw a5, 0x40(a0)
#  jal ra, delay
#  sw a4, 0x40(a0)
#  jal ra, delay
#  sw a3, 0x40(a0)
#  jal ra, delay
#  j loop
#
#delay:
#  lui t0, 0xDCC
#delay_loop:
#  addi t0, t0,-1
#  bne t0, zero, delay_loop
#  sw zero, 0x40(a0)
#  jalr zero, 0(ra)

