  .section .text
   .global enable_fpu
   .type enable_fpu, @function
   enable_fpu:
       li t0, 0x00003000  // Set bits 12 and 13 to enable FPU 0x2000 instead????
       csrs mstatus, t0 
       ret