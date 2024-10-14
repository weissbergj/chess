.section ".text.start"

.globl _start
_start:
.cfi_startproc
.cfi_undefined ra       # tell gdb this is entry point, has no caller
    lui   sp,0x50000    # init sp to top 0x50000000 (stack grows down)
    jal   main

    hang: j hang        # backstop at end of instructions
.cfi_endproc
