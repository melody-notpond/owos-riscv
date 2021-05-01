#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H

#define PAGE_SIZE 4096

// alloc(unsigned long long) -> void*
// Returns a zeroed out pointer to consecutive pages in memory.
void* alloc(unsigned long long page_count);

// dealloc(void*) -> void
// Deallocates a pointer allocated by alloc.
void dealloc(void* ptr);

#endif /* KERNEL_MEMORY_H */

