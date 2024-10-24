/* File: test_backtrace_malloc.c
 * -----------------------------
 * TODO: add your file header comment here
 */
#include "assert.h"
#include "backtrace.h"
#include "malloc.h"
#include "printf.h"
#include <stdint.h>
#include "strings.h"
#include "timer.h"
#include "uart.h"

void heap_dump(const char *label); // available in malloc.c but not public interface

static void function_A(int nframes);
static void function_B(int nframes);

static void check_backtrace(int nframes) {
    frame_t f[nframes];
    int frames_filled = backtrace_gather_frames(f, nframes);

    assert(frames_filled <= nframes);
    printf("Backtrace containing %d frame(s):\n", frames_filled);
    backtrace_print_frames(f, frames_filled);
    printf("\n");
}

static void function_A(int nframes) {
    check_backtrace(nframes);
}

static void function_B(int nframes) {
    function_A(nframes);
}

static int recursion(int n) {
    printf("\nEnter call recursion(%d):\n", n);
    backtrace_print();
    if (n == 0) {
        return 0;
    } else if (n % 2 == 0) {  // even
        return 2 * recursion(n-1);
    } else {                   // odd
        return 1 + recursion(n-1);
    }
}

static void test_backtrace(void) {
    function_B(1);  // grab only topmost frame
    function_B(6);  // now grab several
    recursion(4);
}

static void test_heap_dump(void) {
    heap_dump("Empty heap");

    int *p = malloc(sizeof(int));
    *p = 0;
    heap_dump("After p = malloc(4)");

    char *q = malloc(16);
    memcpy(q, "aaaaaaaaaaaaaaa", 16);
    heap_dump("After q = malloc(16)");

    free(p);
    heap_dump("After free(p)");

    free(q);
    heap_dump("After free(q)");
}

static void test_heap_simple(void) {
    // allocate a string and array of ints
    // assign to values, check, then free
    const char *alphabet = "abcdefghijklmnopqrstuvwxyz";
    int len = strlen(alphabet);

    char *str = malloc(len + 1);
    memcpy(str, alphabet, len + 1);

    int n = 10;
    int *arr = malloc(n*sizeof(int));
    for (int i = 0; i < n; i++) {
        arr[i] = i;
    }

    assert(strcmp(str, alphabet) == 0);
    free(str);
    assert(arr[0] == 0 && arr[n-1] == n-1);
    free(arr);
}

static void test_heap_oddballs(void) {
    // test oddball cases
    char *ptr;

    ptr = malloc(900000000); // request too large to fit
    assert(ptr == NULL); // should return NULL if cannot service request
    heap_dump("After reject too-large request");

    ptr = malloc(0); // legal request, though weird
    heap_dump("After malloc(0)");
    free(ptr);

    free(NULL); // legal request, should do nothing
    heap_dump("After free(NULL)");
}

static void test_heap_multiple(void) {
    // array of dynamically-allocated strings, each
    // string filled with repeated char, e.g. "A" , "BB" , "CCC"
    // Examine each string, verify expected contents intact.

    int n = 8;
    char *arr[n];

    for (int i = 0; i < n; i++) {
        int num_repeats = i + 1;
        char *ptr = malloc(num_repeats + 1);
        memset(ptr, 'A' - 1 + num_repeats, num_repeats);
        ptr[num_repeats] = '\0';
        arr[i] = ptr;
    }
    heap_dump("After all allocations");
    for (int i = n-1; i >= 0; i--) {
        int len = strlen(arr[i]);
        char first = arr[i][0], last = arr[i][len -1];
        assert(first == 'A' - 1 + len);  // verify payload contents
        assert(first == last);
        free(arr[i]);
    }
    heap_dump("After all frees");
}

void test_heap_leaks(void) {
    // This function allocates blocks which are never freed.
    // Leaks will be reported if doing the Valgrind extension, but otherwise
    // they are harmless/silent
    char *ptr;

    ptr = malloc(9); // leaked
    ptr = malloc(5);
    free(ptr);
    ptr = malloc(107); // leaked
    malloc_report();
}

void test_heap_redzones(void) {
    // DO NOT ATTEMPT THIS TEST unless your heap has red zone protection!
    char *ptr;

    ptr = malloc(9);
    memset(ptr, 'a', 9); // write into payload
    free(ptr); // ptr is OK

    ptr = malloc(5);
    ptr[-1] = 0x45; // write before payload
    free(ptr);      // ptr is NOT ok

    ptr = malloc(12);
    ptr[13] = 0x45; // write after payload
    free(ptr);      // ptr is NOT ok
}

void hijack(void) {
    printf("\nHuh? How did we get here? No one calls this function!\n");
    timer_delay(100);
}

void overflow(int num_beyond, void *val) {
    // WARNING: buggy function writes beyond its stack buffer
    void *array[3];

    for (int i = 0; i < 3 + num_beyond; i++) {
        array[i] = val;
    }
    printf("Wrote %d values beyond bufsize at address %p\n", num_beyond, array);
}

void test_stack_protector(void) {
    // calls buggy function overflow which will write past end of stack buffer
    // If StackGuard is enabled, should halt and report stack smashing
    // If not enabled, overflow will do wacky things, consequence differs
    // based on what values overwrote the stack housekeeping data

    void *val;  // try different values to see change in consequence
    val = NULL;
    //val = (void *)0x40000010;
    //val = (void *)0x10000;
    //val = hijack;

    printf("\nCall buggy function that will overflow stack buffer\n");
    for (int i = 1; i < 10; i++) {
        overflow(i, val);
    }
}

void main(void) {
    uart_init();
    uart_putstring("Start execute main() in test_backtrace_malloc.c\n");

    test_backtrace();
    // test_stack_protector(); // Selectively uncomment when ready to test this

    test_heap_dump();
    test_heap_simple();
    test_heap_oddballs();
    test_heap_multiple();
    test_heap_leaks();

    //test_heap_redzones(); // DO NOT USE unless you have implemented red zone protection!
    uart_putstring("\nSuccessfully finished executing main() in test_backtrace_malloc.c\n");
}
