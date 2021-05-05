#include "memory.h"
#include "uart.h"

// TODO: figure out how to make this dependent on how much memory is detected
#define HEAP_SIZE 0x100000

// Represents a page.
typedef unsigned char page_t[PAGE_SIZE];

// Heap bottom
extern struct s_malloc_pointer_header heap_bottom;

// Pages bottom
extern page_t pages_bottom;
page_t* pages_start = &pages_bottom;

enum {
    PAGE_ALLOC_BYTE_FREE = 0,
    PAGE_ALLOC_BYTE_USED = 1,
    PAGE_ALLOC_BYTE_LAST = 2,
};

// init_heap_metadata(void) -> void
// Initialised the heap by allocating space for page metadata.
void init_heap_metadata() {
    unsigned long long page_count = (HEAP_SIZE + PAGE_SIZE - 1) / PAGE_SIZE;
    pages_start += page_count;
    volatile unsigned long long* ptr = (unsigned long long*) &heap_bottom;

    for (; ptr < (unsigned long long*) pages_start; ptr++) {
        *ptr = 0;
    }
}

// is_free(page_t*) -> char
// Checks if a page is free. Returns true if free.
char is_free(page_t* ptr) {
    char* cp = ((char*) &pages_bottom) + (((char*) ptr) - (char*) pages_start) / PAGE_SIZE;
    return *cp == PAGE_ALLOC_BYTE_FREE;
}

// is_used(page_t*) -> char
// Checks if a page is used. Returns true if used.
char is_used(page_t* ptr) {
    char* cp = ((char*) &pages_bottom) + (((char*) ptr) - (char*) pages_start) / PAGE_SIZE;
    return *cp != PAGE_ALLOC_BYTE_FREE;
}

// is_last(page_t*) -> char
// Checks if a page is the last page in an allocation. Returns true if that is the case.
char is_last(page_t* ptr) {
    char* cp = ((char*) &pages_bottom) + (((char*) ptr) - (char*) pages_start) / PAGE_SIZE;
    return (*cp & PAGE_ALLOC_BYTE_LAST) != 0;
}

// alloc_page(unsigned long long) -> void*
// Returns a zeroed out pointer to consecutive pages in memory.
void* alloc_page(unsigned long long page_count) {
    if (page_count == 0)
        return (void*) 0;

    page_t* ptr = pages_start;
    page_t* pages_end = pages_start + HEAP_SIZE;

    // Find a pointer
    for (; ptr < pages_end; ptr++) {
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
                char* cp = ((char*) &pages_bottom) + (((char*) ptr) - (char*) pages_start) / PAGE_SIZE;
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

// dealloc_page(void*) -> void
// Deallocates a pointer allocated by alloc.
void dealloc_page(void* ptr) {
    page_t* page_ptr = (page_t*) ptr;
    char* cp = ((char*) &pages_bottom) + (((char*) page_ptr) - (char*) pages_start) / PAGE_SIZE;

    while (!is_last(page_ptr)) {
        // Mark pages as free
        *cp = PAGE_ALLOC_BYTE_FREE;
        page_ptr++;
        cp++;
    }

    // Mark last page as free
    *cp = PAGE_ALLOC_BYTE_FREE;
}

// malloc(unsigned long int) -> void*
// Allocates a small piece of memory
void* malloc(unsigned long int n) {
    if (n == 0)
        return (void*) 0;

    unsigned long long size = n + sizeof(struct s_malloc_pointer_header);
    struct s_malloc_pointer_header* ptr = &heap_bottom;

    while (ptr < (struct s_malloc_pointer_header*) &pages_bottom) {
        if (!ptr->used && (ptr->size == 0 || n <= ptr->size)) {
            if (ptr->size == 0)
                ptr->size = n;
            if (ptr->next == 0)
                ptr->next = ((void*) ptr) + size;
            ptr->used = 1;
            return ptr + 1;
        }

        if (ptr->next != 0)
            ptr = ptr->next;
        else break;
    }

    if (ptr >= (struct s_malloc_pointer_header*) &pages_bottom)
        uart_puts("[malloc] Out of memory!\n");

    return (void*) 0;
}

// free(void*) -> void
// Frees a piece of memory allocated by malloc.
void free(void* ptr) {
    struct s_malloc_pointer_header* header = ptr - sizeof(struct s_malloc_pointer_header);
    if (header->used) {
        header->used = 0;
    }
}

// memcpy(void*, const void*, unsigned long int) -> void*
// Copys the data from one pointer to another.
void* memcpy(void* dest, const void* src, unsigned long int n) {
    unsigned char* d1 = dest;
    const unsigned char* s1 = src;
    unsigned char* end = dest + n;

    for (; d1 < end; d1++, s1++) {
        *d1 = *s1;
    }

    return dest;
}
