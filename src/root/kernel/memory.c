#include "memory.h"

// TODO: figure out how to make this dependent on how much memory is detected
#define HEAP_SIZE 0x100000

typedef unsigned char page_t[PAGE_SIZE];

// Heap bottom
extern page_t heap_bottom;
page_t* heap_start = &heap_bottom;

enum {
    PAGE_ALLOC_BYTE_FREE = 0,
    PAGE_ALLOC_BYTE_USED = 1,
    PAGE_ALLOC_BYTE_LAST = 2,
};

// init_heap_metadata(void) -> void
// Initialised the heap by allocating space for page metadata.
void init_heap_metadata() {
    unsigned long long page_count = (HEAP_SIZE + PAGE_SIZE - 1) / PAGE_SIZE;
    heap_start += page_count;
    volatile unsigned long long* ptr = (unsigned long long*) &heap_bottom;

    for (; ptr < (unsigned long long*) heap_start; ptr++) {
        *ptr = 0;
    }
}

char is_free(page_t* ptr) {
    return *(((char*) &heap_bottom) + (ptr - heap_start) / PAGE_SIZE) == PAGE_ALLOC_BYTE_FREE;
}

char is_used(page_t* ptr) {
    return *(((char*) &heap_bottom) + (ptr - heap_start) / PAGE_SIZE) != PAGE_ALLOC_BYTE_FREE;
}

char is_last(page_t* ptr) {
    return *(((char*) &heap_bottom) + (ptr - heap_start) / PAGE_SIZE) == PAGE_ALLOC_BYTE_LAST;
}

// alloc(unsigned long long) -> void*
// Returns a zeroed out pointer to consecutive pages in memory.
void* alloc(unsigned long long page_count) {
    page_t* ptr = heap_start;

    // Find a pointer
    for (; ptr < ((page_t*) heap_start) + HEAP_SIZE; ptr++) {
        if (is_free(ptr)) {
            // Check if consecutive pages are free
            char free = 1;
            for (unsigned long long i = 1; i < page_count; i++) {
                if (is_used(ptr + i)) {
                    free = 0;
                    break;
                }
            }

            if (free) {
                // Clear pages
                unsigned long long* big_ptr = (unsigned long long*) ptr;
                for (; big_ptr < (unsigned long long*) (((void*) (ptr + page_count)) + 1); big_ptr++) {
                    *big_ptr = 0;
                }

                // Mark pages as used
                char* cp = ((char*) &heap_bottom) + (ptr - heap_start) / PAGE_SIZE;
                for (page_t* p = ptr; p < ptr + page_count; p++, cp++) {
                    if (p == ptr + page_count - 1)
                        *cp = PAGE_ALLOC_BYTE_LAST;
                    else
                        *cp = PAGE_ALLOC_BYTE_USED;
                }

                return (void*) ptr;
            }
        }
    }

    // No pointer was found; return null
    return (void*) 0;
}

// dealloc(void*) -> void
// Deallocates a pointer allocated by alloc.
void dealloc(void* ptr) {
    page_t* page_ptr = (page_t*) ptr;

    char* cp = ((char*) &heap_bottom) + (page_ptr - heap_start) / PAGE_SIZE;
    while (!is_last(page_ptr)) {
        // Mark pages as free
        *cp = PAGE_ALLOC_BYTE_FREE;
        page_ptr++;
        cp++;
    }
    *cp = PAGE_ALLOC_BYTE_FREE;
}

