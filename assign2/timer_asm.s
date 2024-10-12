/* File: timer_asm.s
 * ------------------
 * ***** TODO: add your file header comment here *****
 */

.attribute arch, "rv64imac_zicsr"

.globl timer_get_ticks
timer_get_ticks:
    # Read the lower 32 bits of the time CSR
    csrr a0, time

    # for 64 bits, do the following:
    # csrr a1, timeh
    # slli a1, a1, 32
    # or a0, a0, a1

    ret
