#include "memory.h"

#define HEAP_SIZE 0x1000

typedef unsigned char page_t[PAGE_SIZE];

// Heap bottom
extern page_t heap_bottom;

enum {
    PAGE_ALLOC_BYTE_FREE = 0,
    PAGE_ALLOC_BYTE_USED = 1,
    PAGE_ALLOC_BYTE_LAST = 2,
};

char is_free(page_t* ptr) {
    return (*ptr)[PAGE_SIZE - 1] == 0;
}

char is_used(page_t* ptr) {
    return (*ptr)[PAGE_SIZE - 1] != 0;
}

// alloc(unsigned long long) -> void*
// Returns a zeroed out pointer to consecutive pages in memory.
void* alloc(unsigned long long page_count) {
    page_t* ptr = &heap_bottom;

    // Find a pointer
    for (; ptr < ((page_t*) &heap_bottom) + HEAP_SIZE; ptr++) {
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

                // Set last byte
                for (unsigned long long i = 0; i < page_count; i++) {
                    (*ptr)[PAGE_SIZE - 1] = 1 << (i + 1 == page_count);
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

    while ((*page_ptr)[PAGE_SIZE - 1] != PAGE_ALLOC_BYTE_LAST) {
        (*page_ptr)[PAGE_SIZE - 1] = 0;
        page_ptr++;
    }

    (*page_ptr)[PAGE_SIZE - 1] = 0;
}

