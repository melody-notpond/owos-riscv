#include "memory.h"
#include "uart.h"

// TODO: figure out how to make this dependent on how much memory is detected
#define HEAP_SIZE 0x100000

// Header for malloc allocations.
struct s_malloc_pointer_header {
    unsigned long long size;
    struct s_malloc_pointer_header* next;
    unsigned char used;
};

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
    // Don't allocate zero sized memory
    if (n == 0)
        return (void*) 0;

    // Calculate size with padding
    unsigned long long size = n + sizeof(struct s_malloc_pointer_header);
    size = (size + 7) & ~7;

    // Iterate until the end of the heap is reached
    struct s_malloc_pointer_header* ptr = &heap_bottom;
    while (ptr + size < (struct s_malloc_pointer_header*) &pages_bottom) {
        // Check if the current header is used
        if (!ptr->used && (ptr->size == 0 || n <= ptr->size)) {
            // Update metadata
            if (ptr->size == 0)
                ptr->size = n;
            if (ptr->next == 0)
                ptr->next = ((void*) ptr) + size;
            ptr->used = 1;
            return ptr + 1;
        }

        // Get next pointer
        if (ptr->next != 0)
            ptr = ptr->next;
        else break;
    }

    // Error message on out of memory
    if (ptr >= (struct s_malloc_pointer_header*) &pages_bottom)
        uart_printf("[malloc] Out of memory!\nAttempted to load address 0x%p\n", ptr);
    return (void*) 0;
}

// realloc(void*, unsigned long int) -> void*
// Reallocates a piece of memory, returning the new pointer.
void* realloc(void* ptr, unsigned long int n) {
    struct s_malloc_pointer_header* header = ptr - sizeof(struct s_malloc_pointer_header);
    if (header->size >= n)
        return ptr;

    void* new = malloc(n);
    if (new == (void*) 0) {
        free(ptr);
        return new;
    }

    memcpy(new, ptr, header->size);
    free(ptr);
    return new;
}

// free(void*) -> void
// Frees a piece of memory allocated by malloc.
void free(void* ptr) {
    if (ptr == (void*) 0)
        return;

    struct s_malloc_pointer_header* header = ptr - sizeof(struct s_malloc_pointer_header);
    if (header->used) {
        header->used = 0;
    }
}

// _sizeof(void*) -> unsigned long long
// Returns the size of a pointer allocated by malloc.
unsigned long long _sizeof(void* ptr) {
    return (((struct s_malloc_pointer_header*) ptr) - 1)->size;
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

// memset(void*, int, unsigned long int) -> void*
// Sets a value over a space. Returns the original pointer.
void* memset(void* p, int i, unsigned long int n) {
    unsigned char c = i;
    for (unsigned char* p1 = p; (void*) p1 < p + n; p1++) {
        *p1 = c;
    }
    return p;
}

// memsize() -> unsigned long long
// Returns the size of the heap in bytes.
unsigned long long memsize() {
    return ((unsigned long long) &pages_bottom) - ((unsigned long long) &heap_bottom);
}

// memfree() -> unsigned long long
// Returns the free space on the kernel heap.
unsigned long long memfree() {
    unsigned long long free = memsize();
    for (struct s_malloc_pointer_header* p = &heap_bottom; p && (void*) p < (void*) &pages_bottom; p = p->next) {
        if (p->used)
            free -= p->size;
    }
    return free;
}
