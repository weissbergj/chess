/* File: backtrace_asm.s
 * ---------------------
 * ***** This file backtraces in assembly *****
 */

.globl backtrace_get_fp
backtrace_get_fp:
    mv a0, fp       # Return current frame pointer
    # ld a0, 0(fp)  # Old: Load saved (previous) frame pointer
    ret