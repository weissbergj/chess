/* File: larson.s
 * --------------
 * ***** TODO: add your file header comment here *****
 */

/*
 * The code below is the blink program from Lab 1.
 * Modify the code to implement the larson scanner for Assignment 1.
 * Be sure to use GPIO pins PB0-PB3 (or PB0-PB7) for your LEDs.
 */

    lui     a0,0x2000        # a0 holds base addr PB group = 0x2000000
    li      a1,0x1111        # we create the binary 0001 x 4 = 0x1111
    sw      a1,0x30(a0)      # config PB0/1/2/3 as outputs instead of inputs by storing 0x1111

    addi    a2,a2,15         # create a value a2 that will turn on the pins via binary 1111


    loop:
      xori    a2,a2,15       # invert a1 for all binary 1
      sw      a2,0x40(a0)    # set data value of PB0/1/2/3 to a2; this tells the pins to emit 3.3V when 1 or 0V when 0
      lui     a3,0x4500      # a3 = init countdown value
   
    delay:
      addi    a3,a3,-1       # decrement a3
      bne     a3,zero,delay  # keep counting down until a3 is zero

      j       loop           # back to top of outer loop

