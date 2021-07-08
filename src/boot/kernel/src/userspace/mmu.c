#include "mmu.h"
#include "../drivers/uart/uart.h"

// create_mmu_top() -> mmu_level_1_t*
// Creates an MMU data structure.
mmu_level_1_t* create_mmu_top() {
    mmu_level_1_t* config = alloc_page(1);
    return config;
}

mmu_level_3_t* walk_mmu_and_get_pointer_to_pointer(mmu_level_1_t* top, void* virtual, int create_pages, unsigned char flags) {
    flags &= ~(MMU_FLAG_READ | MMU_FLAG_WRITE | MMU_FLAG_EXEC | MMU_FLAG_ACCESSED | MMU_FLAG_DIRTY);

    // Top to level 2
    unsigned long long i = (((unsigned long long) virtual) >> 12) & 0x1ff;
    if (top[i].addr == (void*) 0) {
        if (create_pages) {
            top[i].raw = ((unsigned long long) alloc_page(1)) >> 2;
            top[i].raw |= flags | MMU_FLAG_VALID;
        } else {
            return (void*) 0;
        }
    } else if ((top[i].raw & 1) != MMU_FLAG_VALID)
        return (void*) 0;

    // Level 2 to level 3
    mmu_level_2_t* level2 = MMU_UNWRAP(2, top[i]);
    i = (((unsigned long long) virtual) >> 21) & 0x1ff;
    if (level2[i].addr == (void*) 0) {
        if (create_pages) {
            level2[i].raw = ((unsigned long long) alloc_page(1)) >> 2;
            level2[i].raw |= flags | MMU_FLAG_VALID;
        } else {
            return (void*) 0;
        }
    } else if ((level2[i].raw & 1) != MMU_FLAG_VALID)
        return (void*) 0;

    // Get page
    mmu_level_3_t* level3 = MMU_UNWRAP(3, level2[i]);
    i = (((unsigned long long) virtual) >> 30) & 0x1ff;
    return level3 + i;
}

// map_mmu(mmu_level_1_t*, void*, void*, char) -> void
// Maps a virtual address to a physical address.
void map_mmu(mmu_level_1_t* top, void* virtual, void* physical, char flags) {
    // Align addresses to the largest 4096 byte boundary less than the address
    physical = (void*) (((unsigned long long) physical) & ~0xfff);
    virtual = (void*) (((unsigned long long) virtual) & ~0xfff);

    // Walk mmu and create pages along the way
    mmu_level_3_t* level3 = walk_mmu_and_get_pointer_to_pointer(top, virtual, 1, flags);

    if (level3 == (void*) 0) {
        uart_printf("[map_mmu] Error mapping virtual address %p to physical address %p!\n", virtual, physical);
        return;
    } else if (level3->addr != (void*) 0) {
        uart_printf("[map_mmu] Warning: %p is already mapped to %p; not remapping to %p.\n", virtual, MMU_UNWRAP(4, *level3), physical);
        return;
    }

    level3->raw = (unsigned long long) physical >> 2;

    // In addition to the flags provided by the standard, the 8th and 9th bits are reserved for software use
    // In our case, the 8th bit is used to keep track of whether the memory location was allocated with alloc_page().
    level3->raw &= ~0x100;
    level3->raw |= (0b00011111 & flags) | MMU_FLAG_VALID;
}

// alloc_page_mmu(mmu_level_1_t*, void*, char) -> void*
// Allocates a new page to map to a given virtual address. Returns the physical address
void* alloc_page_mmu(mmu_level_1_t* top, void* virtual, char flags) {
    // Align addresses to the largest 4096 byte boundary less than the address
    virtual = (void*) (((unsigned long long) virtual) & ~0xfff);

    // Walk mmu and create pages along the way
    mmu_level_3_t* level3 = walk_mmu_and_get_pointer_to_pointer(top, virtual, 1, flags);

    if (level3 == (void*) 0) {
        uart_printf("[alloc_page_mmu] Error allocating a page for virtual address %p!\n", virtual);
        return (void*) 0;
    } else if (level3->addr != (void*) 0) {
        void* physical = MMU_UNWRAP(4, *level3);
        uart_printf("[alloc_page_mmu] Warning: %p is already mapped to %p; not remapping to a fresh page.\n", virtual, physical);
        return physical;
    }

    void* physical = alloc_page(1);
    level3->raw = (((unsigned long long) physical) & ~0xfff) >> 2;

    // In addition to the flags provided by the standard, the 8th and 9th bits are reserved for software use
    // In our case, the 8th bit is used to keep track of whether the memory location was allocated with alloc_page().
    level3->raw |= 0x100;
    level3->raw |= (0b00011111 & flags) | MMU_FLAG_VALID;
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
    if (level3[k].raw & 0x100)
        dealloc_page(MMU_UNWRAP(4, level3[k]));

    // Unmap
    level3[k].addr = 0;
}

// clean_mmu_mappings(mmu_level_1_t*) -> void
// Deallocates all pages associated with an MMU structure.
void clean_mmu_mappings(mmu_level_1_t* top) {
    if (top == (void*) 0)
        return;

    for (int i = 0; i < (int) (PAGE_SIZE / sizeof(void*)); i++) {
        mmu_level_2_t* level2 = MMU_UNWRAP(2, top[i]);
        if (level2 == (void*) 0)
            continue;

        for (int j = 0; j < (int) (PAGE_SIZE / sizeof(void*)); j++) {
            mmu_level_3_t* level3 = MMU_UNWRAP(3, level2[j]);
            if (level3 == (void*) 0)
                continue;

            for (int k = 0; k < (int) (PAGE_SIZE / sizeof(void*)); k++) {
                if (level3[k].raw & 0x100)
                    dealloc_page(MMU_UNWRAP(4, level3[k]));
            }

            dealloc_page(level3);
        }

        dealloc_page(level2);
    }

    dealloc_page(top);
}
