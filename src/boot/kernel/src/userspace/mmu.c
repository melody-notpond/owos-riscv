#include "mmu.h"

// create_mmu_top() -> mmu_level_1_t*
// Creates an MMU data structure.
mmu_level_1_t* create_mmu_top() {
    mmu_level_1_t* config = alloc_page(1);
    return config;
}

mmu_level_3_t* walk_mmu_and_get_pointer_to_pointer(mmu_level_1_t* top, void* virtual, int create_pages) {
    if (top == (void*) 0)
        return (void*) 0;

    // Top to level 2
    unsigned long long i = (((unsigned long long) virtual) >> 30) & 0x1ff;
    if (top[i].addr == (void*) 0) {
        if (create_pages) {
            top[i].raw = ((unsigned long long) alloc_page(1)) >> 2;
            top[i].raw |= MMU_FLAG_VALID;
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
            level2[i].raw |= MMU_FLAG_VALID;
        } else {
            return (void*) 0;
        }
    } else if ((level2[i].raw & 1) != MMU_FLAG_VALID)
        return (void*) 0;

    // Get page
    mmu_level_3_t* level3 = MMU_UNWRAP(3, level2[i]);
    i = (((unsigned long long) virtual) >> 12) & 0x1ff;
    return level3 + i;
}

// premap_mmu(mmu_level_1_t*, void*) -> void
// Walks an mmu page table and allocates the missing entries on the way to the address that would be mapped to the virtual address given without allocating an address to the virtual address.
void premap_mmu(mmu_level_1_t* top, void* virtual) {
    walk_mmu_and_get_pointer_to_pointer(top, virtual, 1);
}

// walk_mmu(mmu_level_1_t*, void*) -> mmu_level_3_t
// Walks an mmu page table and returns the physical address associated with the given virtual address. Returns null if unmapped.
mmu_level_3_t walk_mmu(mmu_level_1_t* top, void* virtual) {
    mmu_level_3_t* physical_ptr = walk_mmu_and_get_pointer_to_pointer(top, virtual, 0);
    if (physical_ptr == (void*) 0)
        return (mmu_level_3_t) { 0 };
    return *physical_ptr;
}

// map_mmu(mmu_level_1_t*, void*, void*, char) -> int
// Maps a virtual address to a physical address.
int map_mmu(mmu_level_1_t* top, void* virtual, void* physical, char flags) {
    // Align addresses to the largest 4096 byte boundary less than the address
    physical = (void*) (((unsigned long long) physical) & ~0xfff);
    virtual = (void*) (((unsigned long long) virtual) & ~0xfff);

    // Walk mmu and create pages along the way
    mmu_level_3_t* level3 = walk_mmu_and_get_pointer_to_pointer(top, virtual, 1);

    if (level3 == (void*) 0) {
        return -1;
    } else if (level3->addr != (void*) 0) {
        return -1;
    }

    level3->raw = ((unsigned long long) physical) >> 2;

    // In addition to the flags provided by the standard, the 8th and 9th bits are reserved for software use
    // In our case, the 8th bit is used to keep track of whether the memory location was allocated with alloc_page().
    level3->raw &= ~0x100;
    level3->raw |= (0b00111111 & flags) | MMU_FLAG_VALID;
    return 0;
}

// alloc_page_mmu(mmu_level_1_t*, void*, char) -> void*
// Allocates a new page to map to a given virtual address. Returns the physical address
void* alloc_page_mmu(mmu_level_1_t* top, void* virtual, char flags) {
    // Align addresses to the largest 4096 byte boundary less than the address
    virtual = (void*) (((unsigned long long) virtual) & ~0xfff);

    // Walk mmu and create pages along the way
    mmu_level_3_t* level3 = walk_mmu_and_get_pointer_to_pointer(top, virtual, 1);

    if (level3 == (void*) 0) {
        return (void*) 0;
    } else if (level3->addr != (void*) 0) {
        void* physical = MMU_UNWRAP(4, *level3);
        return physical;
    }

    void* physical = alloc_page(1);
    level3->raw = ((unsigned long long) physical) >> 2;

    // In addition to the flags provided by the standard, the 8th and 9th bits are reserved for software use
    // In our case, the 8th bit is used to keep track of whether the memory location was allocated with alloc_page().
    level3->raw |= 0x100;
    level3->raw |= (0b00111111 & flags) | MMU_FLAG_VALID;
    return physical;
}

// mmu_map_range_identity(mmu_level_1_t*, void*, void*, char) -> void
// Maps a range onto itself in an mmu page table.
void mmu_map_range_identity(mmu_level_1_t* top, void* start, void* end, char flags) {
    start = (void*) (((unsigned long long) start) & ~0xfff);
    end = (void*) ((((unsigned long long) end) + PAGE_SIZE - 1) & ~0xfff);

    for (void* p = start; p < end; p += PAGE_SIZE) {
        map_mmu(top, p, p, flags);
    }
}

// TODO: Figure out what addresses for hardware are being used via device trees.
#include "../drivers/virtio/virtio.h"
#include "../interrupts.h"

// mmu_map_kernel(mmu_level_1_t*, fdt_header_t*) -> void
// Maps the kernel onto an mmu page table.
void mmu_map_kernel(mmu_level_1_t* top, fdt_header_t* fdt) {
    extern int text_start;
    extern int data_start;
    extern int ro_data_start;
    extern int sdata_start;
    extern int stack_start;
    extern int heap_bottom;
    extern int pages_bottom;
    extern void* pages_start;

    // Map fdt
    mmu_map_range_identity(top, fdt, ((void*) fdt) + be_to_le(32, fdt->totalsize), MMU_FLAG_GLOBAL | MMU_FLAG_READ);
    mark_pages_as_used(fdt, be_to_le(32, fdt->totalsize));

    // Map kernel
    mmu_map_range_identity(top, &text_start, &data_start,       MMU_FLAG_GLOBAL | MMU_FLAG_READ | MMU_FLAG_EXEC);
    mmu_map_range_identity(top, &data_start, &ro_data_start,    MMU_FLAG_GLOBAL | MMU_FLAG_READ | MMU_FLAG_WRITE);
    mmu_map_range_identity(top, &ro_data_start, &sdata_start,   MMU_FLAG_GLOBAL | MMU_FLAG_READ);
    mmu_map_range_identity(top, &sdata_start, &stack_start,     MMU_FLAG_GLOBAL | MMU_FLAG_READ | MMU_FLAG_WRITE);
    mmu_map_range_identity(top, &stack_start, &heap_bottom,     MMU_FLAG_GLOBAL | MMU_FLAG_READ | MMU_FLAG_WRITE);
    mmu_map_range_identity(top, &heap_bottom, &pages_bottom,    MMU_FLAG_GLOBAL | MMU_FLAG_READ | MMU_FLAG_WRITE);
    mmu_map_range_identity(top, &pages_bottom, pages_start,     MMU_FLAG_GLOBAL | MMU_FLAG_READ | MMU_FLAG_WRITE);

    // Map virtio stuff
    mmu_map_range_identity(top, (void*) VIRTIO_MMIO_BASE, (void*) (VIRTIO_MMIO_TOP + VIRTIO_MMIO_INTERVAL), MMU_FLAG_GLOBAL | MMU_FLAG_READ | MMU_FLAG_WRITE);

    // Map interrupt stuff
    fdt_t devicetree = verify_fdt(fdt);
    void* plic = fdt_find(&devicetree, "plic", (void*) 0);
    PLIC_BASE = fdt_get_node_addr(plic);
    mmu_map_range_identity(top, (void*) PLIC_BASE, (void*) (PLIC_BASE + PLIC_COUNT * 4),                                    MMU_FLAG_GLOBAL | MMU_FLAG_READ | MMU_FLAG_WRITE);
    mmu_map_range_identity(top, (void*) (PLIC_BASE + PLIC_ENABLES_OFFSET), (void*) (PLIC_BASE + PLIC_ENABLES_OFFSET + 1),   MMU_FLAG_GLOBAL | MMU_FLAG_READ | MMU_FLAG_WRITE);
    mmu_map_range_identity(top, (void*) get_context_priority_threshold(1), (void*) (get_context_priority_threshold(1) + 1), MMU_FLAG_GLOBAL | MMU_FLAG_READ | MMU_FLAG_WRITE);

    // Map mmu
    map_mmu(top, top, top, MMU_FLAG_READ | MMU_FLAG_WRITE);
    for (int i = 0; i < (int) (PAGE_SIZE / sizeof(void*)); i++) {
        mmu_level_2_t* level2 = MMU_UNWRAP(2, top[i]);
        if (level2 == (void*) 0)
            continue;

        map_mmu(top, level2, level2, MMU_FLAG_READ | MMU_FLAG_WRITE);

        for (int j = 0; j < (int) (PAGE_SIZE / sizeof(void*)); j++) {
            mmu_level_3_t* level3 = MMU_UNWRAP(3, level2[j]);
            if (level3 == (void*) 0)
                continue;

            map_mmu(top, level3, level3, MMU_FLAG_READ | MMU_FLAG_WRITE);
        }
    }
}

// copy_mmu_globals(mmu_level_1_t*, mmu_level_1_t*) -> void
// Copies the global mappings from one page table to another.
void copy_mmu_globals(mmu_level_1_t* dest, mmu_level_1_t* src) {
    for (int i = 0; i < (int) (PAGE_SIZE / sizeof(void*)); i++) {
        mmu_level_2_t* level2 = MMU_UNWRAP(2, src[i]);
        if (level2 == (void*) 0)
            continue;

        for (int j = 0; j < (int) (PAGE_SIZE / sizeof(void*)); j++) {
            mmu_level_3_t* level3 = MMU_UNWRAP(3, level2[j]);
            if (level3 == (void*) 0)
                continue;

            for (int k = 0; k < (int) (PAGE_SIZE / sizeof(void*)); k++) {
                if (level3[k].raw & MMU_FLAG_GLOBAL) {
                    void* physical = MMU_UNWRAP(4, level3[k]);
                    map_mmu(dest, physical, physical, level3[k].raw & 0xff);
                }
            }
        }
    }

    // Map mmu
    map_mmu(dest, dest, dest, MMU_FLAG_READ | MMU_FLAG_WRITE);
    for (int i = 0; i < (int) (PAGE_SIZE / sizeof(void*)); i++) {
        mmu_level_2_t* level2 = MMU_UNWRAP(2, dest[i]);
        if (level2 == (void*) 0)
            continue;

        map_mmu(dest, level2, level2, MMU_FLAG_READ | MMU_FLAG_WRITE);

        for (int j = 0; j < (int) (PAGE_SIZE / sizeof(void*)); j++) {
            mmu_level_3_t* level3 = MMU_UNWRAP(3, level2[j]);
            if (level3 == (void*) 0)
                continue;

            map_mmu(dest, level3, level3, MMU_FLAG_READ | MMU_FLAG_WRITE);
        }
    }
}

// make_all_global(mmu_level_1_t*) -> void
// Makes all entries of the page table (except for meta entries) global.
void make_all_global(mmu_level_1_t* kernel_mapping) {
    for (int i = 0; i < (int) (PAGE_SIZE / sizeof(void*)); i++) {
        mmu_level_2_t* level2 = MMU_UNWRAP(2, kernel_mapping[i]);
        if (level2 == (void*) 0)
            continue;

        for (int j = 0; j < (int) (PAGE_SIZE / sizeof(void*)); j++) {
            mmu_level_3_t* level3 = MMU_UNWRAP(3, level2[j]);
            if (level3 == (void*) 0)
                continue;

            for (int k = 0; k < (int) (PAGE_SIZE / sizeof(void*)); k++) {
                void* physical = MMU_UNWRAP(4, level3[k]);
                if (physical == (void*) 0 || physical == kernel_mapping)
                    continue;

                // Check if the physical address is part of the mmu metamapping
                char mmu = 0;
                for (int i = 0; i < (int) (PAGE_SIZE / sizeof(void*)); i++) {
                    mmu_level_2_t* level2 = MMU_UNWRAP(2, kernel_mapping[i]);
                    if (level2 == (void*) 0)
                        continue;

                    if (level2 == physical) {
                        mmu = 1;
                        break;
                    }

                    for (int j = 0; j < (int) (PAGE_SIZE / sizeof(void*)); j++) {
                        mmu_level_3_t* level3 = MMU_UNWRAP(3, level2[j]);
                        if (level3 == (void*) 0)
                            continue;

                        if (level3 == physical) {
                            mmu = 1;
                            break;
                        }
                    }

                    if (mmu) break;
                }

                if (!mmu)
                    level3[k].raw |= MMU_FLAG_GLOBAL;
            }
        }
    }
}

// mmu_protect(mmu_level_1_t*, void*, short, int) -> int
// Changes the protection levels on the mmu page. Be careful when setting change_alloc to true.
int mmu_protect(mmu_level_1_t* top, void* virtual, short flags, int change_alloc) {
    virtual = (void*) (((unsigned long long) virtual) & ~0xfff);
    mmu_level_3_t* physical = walk_mmu_and_get_pointer_to_pointer(top, virtual, 0);
    if (physical == (void*) 0)
        return -1;

    if (change_alloc) {
        physical->raw &= ~0x3ff;
        physical->raw |= flags & 0x3ff | MMU_FLAG_VALID;
    } else {
        physical->raw &= ~0xff;
        physical->raw |= flags & 0xff | MMU_FLAG_VALID;
    }

    return 0;
}

// unmap_mmu(mmu_level_1_t*, void*) -> void
// Unmaps a page from the MMU structure.
void unmap_mmu(mmu_level_1_t* top, void* virtual) {
    // Align address to the largest 4096 byte boundary less than the address
    virtual = (void*) (((unsigned long long) virtual) & ~0xfff);

    // Get
    mmu_level_3_t* physical = walk_mmu_and_get_pointer_to_pointer(top, virtual, 0);
    if (physical == (void*) 0)
        return;

    // Deallocate if allocated
    if (physical->raw & 0x100)
        dealloc_page(MMU_UNWRAP(4, *physical));

    // Unmap
    physical->raw = 0;
}

// clean_mmu_mappings(mmu_level_1_t*, char) -> void
// Deallocates all pages associated with an MMU structure.
void clean_mmu_mappings(mmu_level_1_t* top, char force) {
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

            for (int k = 0; k < PAGE_SIZE / sizeof(void*); k++) {
                if ((level3[k].raw & 0x100) && (force || !(level3[k].raw & MMU_FLAG_GLOBAL)))
                    dealloc_page(MMU_UNWRAP(4, level3[k]));
            }

            dealloc_page(level3);
        }

        dealloc_page(level2);
    }

    dealloc_page(top);
}
