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

// Previous backtrace
// int backtrace_gather_frames(frame_t f[], int max_frames) {
//     unsigned long fp = backtrace_get_fp(); 
//     int i = 0;  

//     while (fp != 0 && i < max_frames) { 
//         if ((fp & 0x7) != 0) break; // check for proper addresses

//         f[i].resume_addr = *(unsigned long *)((unsigned char *)(fp) + 8);
//         i++; 

//         fp = *(unsigned long *)(fp); 
//     }

//     return i;  
// }

int backtrace_gather_frames(frame_t f[], int max_frames) {
    unsigned long fp = backtrace_get_fp(); 
    int i = 0;  

    while (fp != 0 && i < max_frames) { 
        // Basic alignment check
        if ((fp & 0x7) != 0) break;

        // Stack range check
        if (fp < 0x4ffff000 || fp >= 0x50000000) break;

        // Get resume address (ra is stored at fp-8)
        unsigned long *ra_ptr = (unsigned long *)((char *)fp - 8);
        if ((unsigned long)ra_ptr < 0x4ffff000) break;

        unsigned long resume_addr = *ra_ptr;

        // Text section check - must be non-zero and in valid range
        if (resume_addr != 0 && resume_addr >= 0x40000000 && resume_addr < 0x40020000) {
            f[i].resume_addr = resume_addr;
            i++; 
        }

        // Get next fp (saved fp is at fp-16)
        unsigned long *next_fp_ptr = (unsigned long *)((char *)fp - 16);
        if ((unsigned long)next_fp_ptr < 0x4ffff000) break;
        
        unsigned long next_fp = *next_fp_ptr;
        if (next_fp <= fp) break;
        fp = next_fp;
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

void __stack_chk_fail(void) {
    void *caller_addr = __builtin_return_address(0);
    if (caller_addr) {
        char labelbuf[128];
        symtab_label_for_addr(labelbuf, sizeof(labelbuf), (uintptr_t)caller_addr);
        printf("Stack smashing detected %p at %s\n", caller_addr, labelbuf);
    } else {
        printf("Stack smashing detected (location unknown)\n");
    }
    mango_abort();
    while(1);
}