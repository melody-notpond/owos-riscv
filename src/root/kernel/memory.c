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
    char* cp = ((char*) &heap_bottom) + (((char*) ptr) - (char*) heap_start) / PAGE_SIZE;
    return *cp == PAGE_ALLOC_BYTE_FREE;
}

char is_used(page_t* ptr) {
    char* cp = ((char*) &heap_bottom) + (((char*) ptr) - (char*) heap_start) / PAGE_SIZE;
    return *cp != PAGE_ALLOC_BYTE_FREE;
}

char is_last(page_t* ptr) {
    char* cp = ((char*) &heap_bottom) + (((char*) ptr) - (char*) heap_start) / PAGE_SIZE;
    return (*cp & PAGE_ALLOC_BYTE_LAST) != 0;
}

// alloc(unsigned long long) -> void*
// Returns a zeroed out pointer to consecutive pages in memory.
void* alloc(unsigned long long page_count) {
    if (page_count == 0)
        return (void*) 0;

    page_t* ptr = heap_start;
    page_t* heap_end = heap_start + HEAP_SIZE;

    // Find a pointer
    for (; ptr < heap_end; ptr++) {
        if (is_free(ptr)) {
            // Check if consecutive pages are free
            char free = 1;
            page_t* end = ptr + page_count;
            for (page_t* p = ptr + 1; p < end; p++) {
                if (is_used(p)) {
                    free = 0;
                    break;
                }
            }

            if (free) {
                // Clear pages
                volatile unsigned long long* big_ptr = (unsigned long long*) ptr;
                for (; big_ptr <= (unsigned long long*) ((void*) end); big_ptr++) {
                    *big_ptr = 0;
                }

                // Mark pages as used
                char* cp = ((char*) &heap_bottom) + (((char*) ptr) - (char*) heap_start) / PAGE_SIZE;
                char* end = cp + page_count;
                for (; cp < end; cp++) {
                    *cp = PAGE_ALLOC_BYTE_USED;
                }
                *(cp - 1) = PAGE_ALLOC_BYTE_LAST;

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

    char* cp = ((char*) &heap_bottom) + (((char*) page_ptr) - (char*) heap_start) / PAGE_SIZE;
    while (!is_last(page_ptr)) {
        // Mark pages as free
        *cp = PAGE_ALLOC_BYTE_FREE;
        page_ptr++;
        cp++;
    }
    *cp = PAGE_ALLOC_BYTE_FREE;
}

void memcpy(void* dest, void* src, unsigned long long n) {
    unsigned char* d1 = dest;
    unsigned char* s1 = src;
    unsigned char* end = dest + n;

    for (; d1 < end; d1++, s1++) {
        *d1 = *s1;
    }
}
