#ifndef KERNEL_MMU_H
#define KERNEL_MMU_H

#include "../lib/memory.h"

#define MMU_UNWRAP(t, a) ((mmu_level_##t##_t*) (((unsigned long long) a.addr) ^ ((unsigned long long) a.flags)))
#define MMU_PAGE_SIZE 4096

// Flags
#define MMU_FLAG_VALID      0b00000001
#define MMU_FLAG_READ       0b00000010
#define MMU_FLAG_WRITE      0b00000100
#define MMU_FLAG_EXEC       0b00001000
#define MMU_FLAG_USER       0b00010000
#define MMU_FLAG_GLOBAL     0b00100000
#define MMU_FLAG_ACCESSED   0b01000000
#define MMU_FLAG_DIRTY      0b10000000

typedef union {
    char flags;
    unsigned long long addr;
} mmu_level_3_t;

typedef union {
    char flags;
    mmu_level_3_t* addr;
} mmu_level_2_t;

typedef union {
    char flags;
    mmu_level_2_t* addr;
} mmu_level_1_t;

// create_mmu_top() -> mmu_level_1_t*
// Creates an MMU data structure.
mmu_level_1_t* create_mmu_top();

// map_mmu(mmu_level_1_t*, void*, void*, char) -> void
// Maps a virtual address to a physical address.
void map_mmu(mmu_level_1_t* top, void* virtual_, void* physical, char flags);

// alloc_page_mmu(mmu_level_1_t*, void*, char) -> void*
// Allocates a new page to map to a given virtual address. Returns the physical address
void* alloc_page_mmu(mmu_level_1_t* top, void* virtual_, char flags);

// unmap_mmu(mmu_level_1_t*, void*) -> void
// Unmaps a page from the MMU structure.
void unmap_mmu(mmu_level_1_t* top, void* virtual_);

// clean_mmu_mappings(mmu_level_1_t*) -> void
// Deallocates all pages associated with an MMU structure.
void clean_mmu_mappings(mmu_level_1_t* top);

#endif /* KERNEL_MMU_H */

