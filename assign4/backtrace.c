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
        if ((fp & 0x7) != 0) break; // check for proper addresses

        f[i].resume_addr = *(unsigned long *)((unsigned char *)(fp) + 8);
        i++; 

        fp = *(unsigned long *)(fp); 
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
    frame_t frame;
    int frames = backtrace_gather_frames(&frame, 1);
    if (frames > 0) printf("Stack overflow at %p\n", (void *)frame.resume_addr);
    else printf("Stack overflow (location unknown)\n");
    mango_abort();
    while(1);
}