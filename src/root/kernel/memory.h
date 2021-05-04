#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H

#define PAGE_SIZE 4096

// init_heap_metadata(void) -> void
// Initialised the heap by allocating space for page metadata.
void init_heap_metadata();

// alloc(unsigned long long) -> void*
// Returns a zeroed out pointer to consecutive pages in memory.
void* alloc(unsigned long long page_count);

// dealloc(void*) -> void
// Deallocates a pointer allocated by alloc.
void dealloc(void* ptr);

void memcpy(void* dest, void* src, unsigned long long n);

#endif /* KERNEL_MEMORY_H */

