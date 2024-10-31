/* File: backtrace_asm.s
 * ---------------------
 * ***** This file backtraces in assembly *****
 */

.globl backtrace_get_fp
backtrace_get_fp:
    ld a0, 0(fp)
    ret