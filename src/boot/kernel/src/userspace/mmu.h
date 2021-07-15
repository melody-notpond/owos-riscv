#ifndef KERNEL_MMU_H
#define KERNEL_MMU_H

#include "../lib/memory.h"

#define MMU_UNWRAP(t, a) ((mmu_level_##t##_t*) ((((a).raw) & ~0x3ff) << 2))
#define MMU_PAGE_SIZE 4096

// Flags
#define MMU_FLAG_VALID      0b000000001
#define MMU_FLAG_READ       0b000000010
#define MMU_FLAG_WRITE      0b000000100
#define MMU_FLAG_EXEC       0b000001000
#define MMU_FLAG_USER       0b000010000
#define MMU_FLAG_GLOBAL     0b000100000
#define MMU_FLAG_ACCESSED   0b001000000
#define MMU_FLAG_DIRTY      0b010000000
#define MMU_FLAG_ALLOCED    0b100000000

typedef void* mmu_level_4_t;

typedef union {
    unsigned long long raw;
    void* addr;
} mmu_level_3_t;

typedef union {
    unsigned long long raw;
    mmu_level_3_t* addr;
} mmu_level_2_t;

typedef union {
    unsigned long long raw;
    mmu_level_2_t* addr;
} mmu_level_1_t;

// create_mmu_top() -> mmu_level_1_t*
// Creates an MMU data structure.
mmu_level_1_t* create_mmu_top();

// premap_mmu(mmu_level_1_t*, void*) -> void
// Walks an mmu page table and allocates the missing entries on the way to the address that would be mapped to the virtual address given without allocating an address to the virtual address.
void premap_mmu(mmu_level_1_t* top, void* _virtual);

// map_mmu(mmu_level_1_t*, void*, void*, char) -> void
// Maps a virtual address to a physical address.
void map_mmu(mmu_level_1_t* top, void* virtual_, void* physical, char flags);

// alloc_page_mmu(mmu_level_1_t*, void*, char) -> void*
// Allocates a new page to map to a given virtual address. Returns the physical address
void* alloc_page_mmu(mmu_level_1_t* top, void* virtual_, char flags);

// walk_mmu(mmu_level_1_t*, void*) -> mmu_level_3_t
// Walks an mmu page table and returns the physical address associated with the given virtual address. Returns null if unmapped.
mmu_level_3_t walk_mmu(mmu_level_1_t* top, void* _virtual);

// mmu_map_range_identity(mmu_level_1_t*, void*, void*, char) -> void
// Maps a range onto itself in an mmu page table.
void mmu_map_range_identity(mmu_level_1_t* top, void* start, void* end, char flags);

// mmu_map_kernel(mmu_level_1_t*) -> void
// Maps the kernel onto an mmu page table.
void mmu_map_kernel(mmu_level_1_t* top);

// copy_mmu_globals(mmu_level_1_t*, mmu_level_1_t*) -> void
// Copies the global mappings from one page table to another.
void copy_mmu_globals(mmu_level_1_t* dest, mmu_level_1_t* src);

// make_all_global(mmu_level_1_t*) -> void
// Makes all entries of the page table (except for meta entries) global.
void make_all_global(mmu_level_1_t* kernel_mapping);

// mmu_protect(mmu_level_1_t*, void*, short, int) -> int
// Changes the protection levels on the mmu page. Be careful when setting change_alloc to true.
int mmu_protect(mmu_level_1_t* top, void* virtual_, short flags, int change_alloc);

// unmap_mmu(mmu_level_1_t*, void*) -> void
// Unmaps a page from the MMU structure.
void unmap_mmu(mmu_level_1_t* top, void* _virtual);

// clean_mmu_mappings(mmu_level_1_t*, char) -> void
// Deallocates all pages associated with an MMU structure.
void clean_mmu_mappings(mmu_level_1_t* top, char force);

#endif /* KERNEL_MMU_H */

