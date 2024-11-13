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

int backtrace_gather_frames(frame_t f[], int max_frames) {
    unsigned long fp = backtrace_get_fp(); 
    int i = 0;  

    while (fp != 0 && i < max_frames) { 
        // Stack range check for fp
        if (fp < 0x4ffff000 || fp >= 0x50000000) break;

        // Try both offsets for return address
        unsigned long resume_addr;
        bool found_valid_addr = false;

        // Try fp-8
        resume_addr = *(unsigned long *)((char *)fp - 8);
        if (resume_addr >= 0x40000000 && resume_addr <= 0x40020000) {
            found_valid_addr = true;
        }

        // Try fp+8 if needed
        if (!found_valid_addr) {
            resume_addr = *(unsigned long *)((char *)fp + 8);
            if (resume_addr >= 0x40000000 && resume_addr <= 0x40020000) {
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
        if (next_fp >= 0x4ffff000 && next_fp < 0x50000000) {
            found_valid_fp = true;
        }

        // Try fp+0 if needed
        if (!found_valid_fp) {
            next_fp = *(unsigned long *)fp;
            if (next_fp >= 0x4ffff000 && next_fp < 0x50000000) {
                found_valid_fp = true;
            }
        }

        if (next_fp == 0xffffffffffffffff) break;

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

long __stack_chk_guard = 0xDEADCCCFCAFEDDBE; // canary value for stack_chk

// Previous
// void __stack_chk_fail(void) {
//     void *caller_addr = __builtin_return_address(0);  // This is safe
//     if (caller_addr) {
//         char labelbuf[128];
//         unsigned long addr = (unsigned long)caller_addr;
        
//         symtab_label_for_addr(labelbuf, sizeof(labelbuf), addr);
        
//         // Skip any leading < characters
//         char *name = labelbuf;
//         while (*name == '<') name++;
        
//         printf("Stack smashing detected 0x%lx at <%s\n", 
//                addr, name);
//     } else {
//         printf("Stack smashing detected (location unknown)\n");
//     }
//     mango_abort();
//     while(1);
// }

// This idk what happened
// void __stack_chk_fail(void) {
//     void *caller_addr = __builtin_return_address(0);
//     if (caller_addr) {
//         char labelbuf[128];
//         unsigned long addr = (unsigned long)caller_addr;
//         unsigned long fn_start = addr & ~0xFF;  // Round down to function start
//         unsigned long offset = addr - fn_start;
        
//         symtab_label_for_addr(labelbuf, sizeof(labelbuf), fn_start);
//         printf("Stack smashing detected 0x%lx at %s\n", 
//                addr, labelbuf);
//     } else {
//         printf("Stack smashing detected (location unknown)\n");
//     }
//     mango_abort();
//     while(1);
// }

// This tells us what address/start to use
// void __stack_chk_fail(void) {
//     void *caller_addr = __builtin_return_address(0);  // This is safe
//     if (caller_addr) {
//         printf("\nDebug:\n");
//         printf("caller_addr = 0x%lx\n", (unsigned long)caller_addr);
        
//         // Try different addresses
//         char label1[128], label2[128], label3[128];
//         unsigned long addr = (unsigned long)caller_addr;
        
//         // 1. Raw address
//         symtab_label_for_addr(label1, sizeof(label1), addr);
//         printf("Raw addr: 0x%lx -> '%s'\n", addr, label1);
        
//         // 2. Function start
//         unsigned long fn_start = addr & ~0xFF;
//         symtab_label_for_addr(label2, sizeof(label2), fn_start);
//         printf("Fn start: 0x%lx -> '%s'\n", fn_start, label2);
        
//         // 3. Try rounding to different boundary
//         unsigned long fn_start2 = addr & ~0xF;
//         symtab_label_for_addr(label3, sizeof(label3), fn_start2);
//         printf("Fn start2: 0x%lx -> '%s'\n", fn_start2, label3);
        
//         printf("Stack smashing detected 0x%lx at %s\n", 
//                addr, label1);
//     } else {
//         printf("Stack smashing detected (location unknown)\n");
//     }
//     mango_abort();
//     while(1);
// }

// This works for first two strangely
// void __stack_chk_fail(void) {
//     void *caller_addr = __builtin_return_address(0);  // This is safe
//     if (caller_addr) {
//         char labelbuf[128];
//         unsigned long addr = (unsigned long)caller_addr;
        
//         // Try different masks to find function start
//         unsigned long fn_start1 = addr & ~0xFF;  // 256-byte boundary
//         unsigned long fn_start2 = addr & ~0x7F;  // 128-byte boundary
//         unsigned long fn_start3 = addr & ~0x3F;  // 64-byte boundary
        
//         // Get labels for each
//         char label1[128], label2[128], label3[128];
//         symtab_label_for_addr(label1, sizeof(label1), fn_start1);
//         symtab_label_for_addr(label2, sizeof(label2), fn_start2);
//         symtab_label_for_addr(label3, sizeof(label3), fn_start3);
        
//         printf("\nDebug:\n");
//         printf("Raw addr: 0x%lx\n", addr);
//         printf("256-byte: 0x%lx -> '%s'\n", fn_start1, label1);
//         printf("128-byte: 0x%lx -> '%s'\n", fn_start2, label2);
//         printf("64-byte:  0x%lx -> '%s'\n", fn_start3, label3);
        
//         // Use the first one for now
//         printf("Stack smashing detected 0x%lx at %s\n", 
//                addr, label1);
//     } else {
//         printf("Stack smashing detected (location unknown)\n");
//     }
//     mango_abort();
//     while(1);
// }

void __stack_chk_fail(void) {
    void *caller_addr = __builtin_return_address(0);
    if (caller_addr) {
        char labelbuf[128];
        unsigned long addr = (unsigned long)caller_addr;
        unsigned long fn_start = addr & ~0x7F;  // 128-byte boundary
        
        symtab_label_for_addr(labelbuf, sizeof(labelbuf), fn_start);
        printf("Stack smashing detected 0x%lx at %s\n", 
               addr, labelbuf);
    } else {
        printf("Stack smashing detected (location unknown)\n");
    }
    mango_abort();
    while(1);
}