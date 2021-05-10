#include "mmu.h"

// create_mmu_top() -> mmu_level_1_t*
// Creates an MMU data structure.
mmu_level_1_t* create_mmu_top() {
    mmu_level_1_t* config = alloc_page(1);
    return config;
}

// map_mmu(mmu_level_1_t*, void*, void*, char) -> void
// Maps a virtual address to a physical address.
void map_mmu(mmu_level_1_t* top, void* virtual, void* physical, char flags) {
    // Align addresses to the largest 4096 byte boundary less than the address
    physical = (void*) (((unsigned long long) physical) & ~0xfff);
    virtual = (void*) (((unsigned long long) virtual) & ~0xfff);

    // Top to level 2
    unsigned long long i = (((unsigned long long) virtual) & 0x7fb0000000) >> 30;
    if (top[i].addr == (void*) 0) {
        top[i].addr = alloc_page(1);
        top[i].flags = (0b00010001 & flags) | 1;
    }

    // Level 2 to level 3
    mmu_level_2_t* level2 = MMU_UNWRAP(2, top[i]);
    i = (((unsigned long long) level2) & 0x003fe00000) >> 21;
    if (level2[i].addr == (void*) 0) {
        level2[i].addr = alloc_page(1);
        level2[i].flags = (0b00010001 & flags) | 1;
    }

    // Set address
    mmu_level_3_t* level3 = MMU_UNWRAP(3, level2[i]);
    i = (((unsigned long long) level2) & 0x00001ff000) >> 12;
    level3[i].addr = (unsigned long long) physical >> 2;

    // In addition to the flags provided by the standard, the 8th and 9th bits are reserved for software use
    // In our case, the 8th bit is used to keep track of whether the memory location was allocated with alloc_page().
    level3[i].addr &= ~0x100;
    level3[i].flags = (0b00011111 & flags) | 1;
}

// alloc_page_mmu(mmu_level_1_t*, void*, char) -> void*
// Allocates a new page to map to a given virtual address. Returns the physical address
void* alloc_page_mmu(mmu_level_1_t* top, void* virtual, char flags) {
    // Get new page
    void* physical = alloc_page(1);

    // Align virtual address to the largest 4096 byte boundary less than the address
    virtual = (void*) (((unsigned long long) virtual) & ~0xfff);

    // Level 1 to level 2
    unsigned long long i = (((unsigned long long) virtual) & 0x7fb0000000) >> 30;
    if (top[i].addr == (void*) 0) {
        top[i].addr = alloc_page(1);
        top[i].flags = (0b00010001 & flags) | 1;
    }

    // Level 2 to level 3
    mmu_level_2_t* level2 = MMU_UNWRAP(2, top[i]);
    i = (((unsigned long long) level2) & 0x003fe00000) >> 21;
    if (level2[i].addr == (void*) 0) {
        level2[i].addr = alloc_page(1);
        level2[i].flags = (0b00010001 & flags) | 1;
    }

    // Set address
    mmu_level_3_t* level3 = MMU_UNWRAP(3, level2[i]);
    i = (((unsigned long long) level2) & 0x00001ff000) >> 12;
    level3[i].addr = (unsigned long long) physical >> 2;

    // In addition to the flags provided by the standard, the 8th and 9th bits are reserved for software use
    // In our case, the 8th bit is used to keep track of whether the memory location was allocated with alloc_page().
    level3[i].addr |= 0x100;
    level3[i].flags = (0b00011111 & flags) | 1;
    return physical;
}

// unmap_mmu(mmu_level_1_t*, void*) -> void
// Unmaps a page from the MMU structure.
void unmap_mmu(mmu_level_1_t* top, void* virtual) {
    // Align address to the largest 4096 byte boundary less than the address
    virtual = (void*) (((unsigned long long) virtual) & ~0xfff);

    // Level 1 to level 2
    unsigned long long i = (((unsigned long long) virtual) & 0x7fb0000000) >> 30;
    if (top[i].addr == (void*) 0) {
        return;
    }

    // Level 2 to level 3
    mmu_level_2_t* level2 = MMU_UNWRAP(2, top[i]);
    unsigned long long j = (((unsigned long long) level2) & 0x003fe00000) >> 21;
    if (level2[j].addr == (void*) 0) {
        return;
    }

    // Get index
    mmu_level_3_t* level3 = MMU_UNWRAP(3, level2[j]);
    unsigned long long k = (((unsigned long long) level2) & 0x00001ff000) >> 12;

    // Deallocate if allocated
    if (level3[k].addr & 0x100) {
        void* physical = (void*) ((level3[k].addr ^ level3[k].flags) << 2);
        dealloc_page(physical);
    }

    // Unmap
    level3[k].addr = 0;
}

// clean_mmu_mappings(mmu_level_1_t*) -> void
// Deallocates all pages associated with an MMU structure.
void clean_mmu_mappings(mmu_level_1_t* top) {
    if (top == (void*) 0)
        return;

    for (int i = 0; i < PAGE_SIZE / sizeof(void*); i++) {
        mmu_level_2_t* level2 = MMU_UNWRAP(2, top[i]);
        if (level2 == (void*) 0)
            continue;

        for (int j = 0; j < PAGE_SIZE / sizeof(void*); j++) {
            mmu_level_3_t* level3 = MMU_UNWRAP(3, level2[j]);
            if (level3 == (void*) 0)
                continue;

            for (int k = 0; k < PAGE_SIZE / sizeof(void*); k++) {
                if (level3[k].addr & 0x100) {
                    void* physical = (void*) ((level3[k].addr ^ level3[k].flags) << 2);
                    dealloc_page(physical);
                }
            }

            dealloc_page(level3);
        }

        dealloc_page(level2);
    }

    dealloc_page(top);
}
