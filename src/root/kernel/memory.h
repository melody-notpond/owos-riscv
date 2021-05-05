#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H

#define PAGE_SIZE 4096

// Header for malloc allocations.
struct s_malloc_pointer_header {
    unsigned long long size;
    struct s_malloc_pointer_header* next;
    unsigned char used;
};

// init_heap_metadata(void) -> void
// Initialised the heap by allocating space for page metadata.
void init_heap_metadata();

// alloc_page(unsigned long long) -> void*
// Returns a zeroed out pointer to consecutive pages in memory.
void* alloc_page(unsigned long long page_count);

// dealloc_page(void*) -> void
// Deallocates a pointer allocated by alloc.
void dealloc_page(void* ptr);

// malloc(unsigned long int) -> void*
// Allocates a small piece of memory
void* malloc(unsigned long int n);

// free(void*) -> void
// Frees a piece of memory allocated by malloc.
void free(void* ptr);

// memcpy(void*, const void*, unsigned long int) -> void*
// Copys the data from one pointer to another.
void* memcpy(void* dest, const void* src, unsigned long int n);

#endif /* KERNEL_MEMORY_H */

