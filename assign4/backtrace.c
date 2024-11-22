/* File: backtrace.c
 * -----------------
 * ***** This file includes backtrace functionality *****
 */
#include "backtrace.h"
#include "mango.h"
#include "printf.h"
#include "symtab.h"

// helper function implemented in file backtrace_asm.s
extern unsigned long backtrace_get_fp(void);

// int backtrace_gather_frames(frame_t f[], int max_frames) {
//     unsigned long fp = backtrace_get_fp(); 
//     int i = 0;  

//     while (fp != 0 && i < max_frames) { 
//         // Stack range check for fp
//         if (fp < 0x40000000 || fp >= 0x50000000) break;

//         // Try both offsets for return address
//         unsigned long resume_addr;
//         bool found_valid_addr = false;

//         // Try fp-8
//         resume_addr = *(unsigned long *)((char *)fp - 8);
//         if (resume_addr > 0) {
//             found_valid_addr = true;
//         }

//         // Try fp+8 if needed
//         if (!found_valid_addr) {
//             resume_addr = *(unsigned long *)((char *)fp + 8);
//             if (resume_addr > 0) {
//                 found_valid_addr = true;
//             }
//         }

//         if (found_valid_addr) {
//             f[i].resume_addr = resume_addr;
//             i++;
//         }

//         // Try both offsets for next frame pointer
//         unsigned long next_fp;
//         bool found_valid_fp = false;

//         // Try fp-16
//         next_fp = *(unsigned long *)((char *)fp - 16);
//         if (next_fp >= 0x40000000 && next_fp < 0x50000000) {
//             found_valid_fp = true;
//         }

//         // Try fp+0 if needed
//         if (!found_valid_fp) {
//             next_fp = *(unsigned long *)fp;
//             if (next_fp >= 0x40000000 && next_fp < 0x50000000) {
//                 found_valid_fp = true;
//             }
//         }

//         if (found_valid_fp) {
//             fp = next_fp;
//         } else {
//             break;
//         }
//     }

//     return i;  
// }

int backtrace_gather_frames(frame_t f[], int max_frames) {
    unsigned long fp = backtrace_get_fp(); 
    int i = 0;  

    while (fp != 0 && i < max_frames) { 
        // Check if fp is aligned
        if ((fp % sizeof(unsigned long)) != 0) {
            break; // Exit if fp is misaligned
        }

        // Try both offsets for return address
        unsigned long resume_addr;
        bool found_valid_addr = false;

        // Try fp-8
        resume_addr = *(unsigned long *)((char *)fp - 8);
        if (resume_addr > 0) {
            found_valid_addr = true;
        }

        // Try fp+8 if needed
        if (!found_valid_addr) {
            resume_addr = *(unsigned long *)((char *)fp + 8);
            if (resume_addr > 0) {
                found_valid_addr = true;
            }
        }

        if (found_valid_addr) {
            f[i].resume_addr = resume_addr;
            i++;
        }

        // Try both offsets for next frame pointer
        unsigned long next_fp;
        bool found_valid_fp = false;

        // Try fp-16
        next_fp = *(unsigned long *)((char *)fp - 16);
        // Check if next_fp is aligned and valid
        if ((next_fp % sizeof(unsigned long)) == 0 && next_fp != 0) {
            found_valid_fp = true;
        }

        // Try fp+0 if needed
        if (!found_valid_fp) {
            next_fp = *(unsigned long *)fp;
            if ((next_fp % sizeof(unsigned long)) == 0 && next_fp != 0) {
                found_valid_fp = true;
            }
        }

        if (found_valid_fp) {
            fp = next_fp;
        } else {
            break;
        }
    }

    return i;  
}



void backtrace_print_frames(frame_t f[], int n) {
    char labelbuf[128];

    for (int i = 0; i < n; i++) {
        symtab_label_for_addr(labelbuf, sizeof(labelbuf), f[i].resume_addr);
        printf("#%d 0x%08lx at %s\n", i, f[i].resume_addr, labelbuf);
    }
}

void backtrace_print(void) {
    int max = 50;
    frame_t arr[max];

    int n = backtrace_gather_frames(arr, max);
    backtrace_print_frames(arr+1, n-1);   // print frames starting at this function's caller
}

long __stack_chk_guard = 0xDD00DDFFCAFE0000; // canary value for stack_chk added null values

void __stack_chk_fail(void) {
    void *caller_addr = __builtin_return_address(0);
    if (caller_addr) {
        char labelbuf[128];
        unsigned long addr = (unsigned long)caller_addr - 8; // note subtracted 8
        
        symtab_label_for_addr(labelbuf, sizeof(labelbuf), addr);
        printf("Stack smashing detected 0x%lx at %s\n", 
               addr, labelbuf);
    } else {
        printf("Stack smashing detected (location unknown)\n");
    }
    mango_abort();
    while(1);
}