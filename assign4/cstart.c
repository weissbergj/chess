/* File: cstart.c
 * --------------
 * _cstart function
 */

extern int main(void);
extern char __bss_start, __bss_end;

void _cstart(void) {
    char *bss = &__bss_start;
    char *bss_end = &__bss_end;

    while (bss < bss_end) {
        *bss++ = 0;
    }

    main();
    while (1) { }
} 