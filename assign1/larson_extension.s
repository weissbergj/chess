/* File: larson.s
 * --------------
 * ***** EXTENSION *****
 */

#
lui a0, 0x2000
li a1, 0x11111111
sw a1, 0x30(a0)

li a2, 0b00000001
li t0, 0b10000000
li t1, 0b00000001


equal:
  sw a2, 0x40(a0)
  jal ra, delay
  sw zero, 0x40(a0)
  slli a2, a2, 1
  bne t0, a2, equal


not_equal:  
  sw a2, 0x40(a0)
  jal ra, delay
  sw zero, 0x40(a0)
  srli a2, a2, 1
  bne a2, t1, not_equal

  j equal


delay:
  lui t2, 0xDCC
delay_loop:
  addi t2, t2, -1
  bne t2, zero, delay_loop
  jalr zero, 0(ra)






#
#lui a0, 0x2000
#li a1, 0x11111111
#sw a1, 0x30(a0)
#
#
#li a2, 2
#li a3, 7
#
#
#loop:
#  sw a2, 0x40(a0)
#  jal ra, delay_off
#  
#  sw a3, 0x40(a0)
#  jal ra, delay_on
#  
#  j loop
#
#
#delay_on:
#  lui t0, 0x50
#delay_loop:
#  addi t0, t0, -1
#  bne t0, zero, delay_loop
#  jalr zero, 0(ra)
#
#delay_off:
#  lui t0, 0x300
#delay_loop_off:
#  addi t0, t0, -1
#  bne t0, zero, delay_loop_off
#  jalr zero, 0(ra)
#
#
#
## instead of initializing all "off" values, initialize only the "on" and then do xori? does this save time if we still ## have to initialize the xori input register that switches the "on" register "off"..?
  


