/* File: malloc.c
 * --------------
 * ***** Malloc implementation *****
 */

#include "malloc.h"
#include "memmap.h"
#include "printf.h"
#include <stddef.h> // for NULL
#include "strings.h"

static int count_allocs, count_frees, total_bytes_requested;
#define roundup(x,n) (((x)+((n)-1))&(~((n)-1)))

void *sbrk(size_t nbytes) {
    // __heap_start, __heap_max symbols are defined in linker script memmap.ld
    static void *cur_heap_end =  &__heap_start;     // IMPORTANT: static

    void *new_heap_end = (char *)cur_heap_end + nbytes;
    if (new_heap_end > (void *)&__heap_max)    // if request would extend beyond heap max
        return NULL;                // reject
    void *prev_heap_end = cur_heap_end;
    cur_heap_end = new_heap_end;
    return prev_heap_end;
}

typedef struct header {
    size_t payload_size;
    int status;          // 0 if free; 1 if in use
} header;

void *malloc(size_t nbytes) {
    if (nbytes == 0) nbytes = 8;
    
    count_allocs++;                              
    total_bytes_requested += nbytes;             
    
    size_t rounded_size = roundup(nbytes, 8);
    size_t total_size = sizeof(header) + rounded_size;
    
    header *current_heap = (header *)&__heap_start;
    void *heap_end = sbrk(0);
    
    while ((void *)current_heap < heap_end) {
        if (current_heap->status == 0 && current_heap->payload_size >= rounded_size) {
            size_t excess = current_heap->payload_size - rounded_size;
            if (excess >= sizeof(header) + 8) {
                header *next = (header *)((char *)current_heap + sizeof(header) + rounded_size);
                next->payload_size = excess - sizeof(header);
                next->status = 0;
                current_heap->payload_size = rounded_size;
            }
            current_heap->status = 1;
            return (void *)(current_heap + 1);
        }
        current_heap = (header *)((char *)current_heap + sizeof(header) + current_heap->payload_size);
    }
    
    header *new_block = sbrk(total_size);
    if (!new_block) {
        count_allocs--;
        total_bytes_requested -= nbytes;
        return NULL;
    }
    
    new_block->payload_size = rounded_size;
    new_block->status = 1;
    return (void *)(new_block + 1);
}

void free(void *ptr) {
    if (!ptr) return;

    count_frees++;
    header *h = ((header *)ptr) - 1;
    h->status = 0;
    
    void *heap_end = sbrk(0);
    header *current_heap = (header *)&__heap_start;
    
    while ((void *)current_heap < heap_end) {
        if (!current_heap->status) {
            header *next = (header *)((char *)current_heap + sizeof(header) + current_heap->payload_size);
            while ((void *)next < heap_end && !next->status) {
                current_heap->payload_size += sizeof(header) + next->payload_size;
                next = (header *)((char *)current_heap + sizeof(header) + current_heap->payload_size);
            }
        }
        current_heap = (header *)((char *)current_heap + sizeof(header) + current_heap->payload_size);
        if ((void *)current_heap >= heap_end) break;
    }
}

void heap_dump(const char *label) {
    int count = 0;
    printf("\n---------- HEAP DUMP (%s) ----------\n", label);
    void *heap_end = sbrk(0);
    printf("Heap segment at %p - %p\n", &__heap_start, heap_end);

    if (heap_end == &__heap_start) {
        printf("Empty heap\n");
        goto print_stats;
    }

    for (header *current_heap = (header *)&__heap_start; (void *)current_heap < heap_end;) {
        printf("\nBlock %d:\n", count++);
        printf("  Header    = %p\n  Payload   = %p\n  Size      = %d bytes\n  Status    = %s\n",
               current_heap, (void *)(current_heap + 1), (int)current_heap->payload_size,
               current_heap->status ? "in use" : "free");
        
        void *next = (char *)current_heap + sizeof(header) + current_heap->payload_size;
        if (next > heap_end) {
            printf("  WARNING: Block extends beyond heap end\n");
            break;
        }
        current_heap = (header *)next;
    }

print_stats:
    printf("----------  END DUMP (%s) ----------\n", label);
    printf("Stats: %d in-use (%d allocs, %d frees), %d  bytes requested\n\n",
           count_allocs - count_frees, count_allocs, count_frees, (int)total_bytes_requested);
}

void malloc_report(void) {
    printf("\n=============================================\n");
    printf("         Mini-Valgrind Malloc Report         \n");
    printf("===============================================\n");
    
    header *current_heap = (header *)&__heap_start;
    void *heap_end = sbrk(0);
    int leaks = 0;
    int leaked_bytes = 0;
    
    while ((void *)current_heap < heap_end) {
        if (current_heap->status == 1) {
            leaks++;
            leaked_bytes += current_heap->payload_size;
            printf("Block at %p of size %d bytes  still allocated\n", 
                   (void *)(current_heap + 1), (int)current_heap->payload_size);
        }
        current_heap = (header *)((char *)current_heap + sizeof(header) + current_heap->payload_size);
    }
    
    printf("\nLeak check: %d blocks leaked, %d bytes lost\n", leaks, leaked_bytes);
    printf("Final stats: %d allocs, %d frees, %d total bytes requested\n\n",
           count_allocs, count_frees, (int)total_bytes_requested);
}

void report_damaged_redzone (void *ptr) {
    printf("\n=============================================\n");
    printf(  " **********  Mini-Valgrind Alert  ********** \n");
    printf(  "=============================================\n");
    printf("Attempt to free address %p that has damaged red zone(s):", ptr);
    /***** TODO EXTENSION: Your code goes here if implementing extension *****/
}