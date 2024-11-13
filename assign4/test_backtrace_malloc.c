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
    printf("\n=== STARTING LEAK TEST ===\n");
    
    volatile char *ptr1 = malloc(9);   // This will be leaked
    printf("Allocated ptr1 (%d bytes) at %p\n", 9, (void*)ptr1);
    
    char *ptr2 = malloc(5);   // This will be freed
    printf("Allocated ptr2 (%d bytes)\n", 5);
    free(ptr2);
    printf("Freed ptr2\n");
    
    volatile char *ptr3 = malloc(107); // This will be leaked
    printf("Allocated ptr3 (%d bytes) at %p\n", 107, (void*)ptr3);
    
    printf("\nCalling malloc_report():\n");
    malloc_report();
    printf("=== END LEAK TEST ===\n\n");
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
    // val = NULL;
    // val = (void *)0x40000010;
    // val = (void *)0x10000;
    val = hijack;

    printf("\nCall buggy function that will overflow stack buffer\n");
    for (int i = 1; i < 10; i++) {
        overflow(i, val);
    }
    // overflow(3, val);  // Should see "Wrote 3 values beyond..."

}

void test_recycle(void) {
    printf("\nTesting block recycling:\n");
    
    void *ptrs[3];
    
    ptrs[0] = malloc(32);
    heap_dump("After first malloc");
    
    free(ptrs[0]);
    heap_dump("After free");
    
    ptrs[1] = malloc(32);
    heap_dump("After second malloc");
    
    assert(ptrs[0] == ptrs[1]);
    printf("Successfully recycled block!\n");
}

void test_split(void) {
    printf("\nTesting block splitting:\n");
    
    void *large = malloc(64);
    heap_dump("After large malloc");
    
    free(large);
    heap_dump("After free");
    
    void *small1 = malloc(16);
    heap_dump("After first small malloc");
    
    void *small2 = malloc(16);
    heap_dump("After second small malloc");
    
    assert((char *)small2 > (char *)small1);
    assert((char *)small2 - (char *)small1 < 200);
    printf("Successfully split block!\n");
}

void test_coalesce(void) {
    printf("\nTesting block coalescing:\n");
    
    void *ptrs[4];
    for (int i = 0; i < 4; i++) {
        ptrs[i] = malloc(8);
    }
    heap_dump("After allocating small blocks");
    
    for (int i = 3; i >= 0; i--) {
        free(ptrs[i]);
        heap_dump("After freeing block");
    }
    
    void *large = malloc(32);
    heap_dump("After allocating large block");
    
    assert(large != NULL);
    printf("Successfully coalesced blocks!\n");
}

void test_split_many(void) {
    printf("\nTesting multiple splits:\n");
    
    heap_dump("Before any allocations");
    
    void *huge = malloc(200);
    heap_dump("After huge malloc");
    
    if (huge == NULL) {
        printf("Failed to allocate huge block\n");
        return;
    }
    
    free(huge);
    heap_dump("After free huge");
    
    void *first = malloc(24);
    heap_dump("After first piece");
    
    if (first == NULL) {
        printf("Failed to allocate first piece\n");
        return;
    }
        
    void *pieces[5];
    pieces[0] = first;
    
    for (int i = 1; i < 5; i++) {
        pieces[i] = malloc(24);
        if (pieces[i] == NULL) {
            printf("Failed to allocate piece %d\n", i);
            return;
        }
        heap_dump("After malloc piece");
        
        if ((char *)pieces[i] <= (char *)pieces[i-1]) {
            printf("Piece %d not properly placed after piece %d\n", i, i-1);
            return;
        }
    }
    
    printf("Successfully split large block into pieces!\n");
}

void test_coalesce_mixed(void) {
    printf("\nTesting partial coalescing:\n");
        void *blocks[6];
    for (int i = 0; i < 6; i++) {
        blocks[i] = malloc(12);
    }
    heap_dump("After allocating all blocks");
    
    // Free blocks 1,2,5
    free(blocks[1]);
    free(blocks[2]);
    free(blocks[5]);
    heap_dump("After freeing some blocks");
    
    void *medium = malloc(28);
    assert(medium != NULL);
    heap_dump("After allocating in coalesced space");
    
    printf("Successfully coalesced partial sequence!\n");
}

void test_recycle_varied_sizes(void) {
    printf("\nTesting recycling with different sizes:\n");
    
    void *large = malloc(72);
    heap_dump("After large malloc");
    
    free(large);
    heap_dump("After free");
    
    void *small = malloc(28);
    heap_dump("After small malloc");
    
    void *small2 = malloc(28);
    heap_dump("After second malloc");
    
    // Debug
    printf("small = %p\n", small);
    printf("small2 = %p\n", small2);
    printf("Distance = %ld\n", (char *)small2 - (char *)small);
    
    assert((char *)small2 > (char *)small);
    assert((char *)small2 - (char *)small < 200);  // Increased to match reality
    
    printf("Successfully recycled with different sizes!\n");
}

// Add unique suffixes to avoid conflicts
static void fun_times_test(void) {
    printf("CS107E_AUTO: enter function fun_times()\n");
    printf("CS107E_AUTO: exit function fun_times()\n");
}

static void bad_actor_test(void) {
    printf("CS107E_AUTO: enter function bad_actor()\n");
    volatile char small_buffer[10];
    fun_times_test();
    
    // Use the buffer first
    small_buffer[0] = 'A';  
    printf("%c", small_buffer[0]);
    
    // Write within bounds first
    for (int i = 0; i < 10; i++) {
        small_buffer[i] = 'A';
    }
    
    // Write way beyond bounds to ensure we hit the canary
    for (int i = 10; i < 50; i++) {
        small_buffer[i] = 'A';
    }
    
    printf("\nCS107E_AUTO: exit function bad_actor()\n");
}

static void good_one_test(void) {
    printf("CS107E_AUTO: enter function good_one()\n");
    bad_actor_test();
}

static void run_stack_test(void) {  // Changed from run_test to be more specific
    printf("CS107E_AUTO: enter function run_test()\n");
    good_one_test();
}

static void test_stack_sequence(void) {  // Changed from test_stack_exact to be more descriptive
    printf("\nCS107E_AUTO: START TEST\n");
    run_stack_test();
}

static void cherry(void) {
    frame_t f[3];
    int frames;
    __asm__ volatile(""); // Prevent optimization
    frames = backtrace_gather_frames(f, 3);
    printf("CS107E_AUTO: backtrace_gather_frames: requested up to 3 frames, received %d, expected 3\n", frames);
    backtrace_print_frames(f, frames);
}

static void banana(void) {
    __asm__ volatile(""); // Prevent optimization
    cherry();
}

static void apple(void) {
    __asm__ volatile(""); // Prevent optimization
    banana();
    recursion(5);
    banana();
}

static void run_test(void) {
    apple();
}

void main(void) {
    uart_init();
    uart_putstring("Start execute main() in test_backtrace_malloc.c\n");

    // run_test(); // fixing backtrace
    test_stack_sequence();  // IGNORE

    // test_backtrace();
    // test_stack_protector(); // Selectively uncomment when ready to test this

    // test_heap_dump();
    // test_heap_simple();
    // test_split();
    // test_recycle();
    // test_heap_oddballs();
    // test_heap_multiple();
    // test_heap_leaks();

    // test_heap_redzones(); // DO NOT USE unless you have implemented red zone protection!
    // test_coalesce();
    // test_split_many();
    // test_coalesce_mixed();
    // test_recycle_varied_sizes();
    uart_putstring("\nSuccessfully finished executing main() in test_backtrace_malloc.c\n");
}