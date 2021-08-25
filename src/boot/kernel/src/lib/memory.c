#include "memory.h"
#include "../userspace/mmu.h"
#include "../drivers/console/console.h"
#include "../drivers/devicetree/tree.h"

unsigned long long HEAP_SIZE;

// Header for malloc allocations.
struct s_malloc_pointer_header {
    unsigned long long size;
    struct s_malloc_pointer_header* next;
};

// Represents a page.
typedef unsigned char page_t[PAGE_SIZE];

struct {
    struct s_malloc_pointer_header* bucket_16;
    struct s_malloc_pointer_header* bucket_32;
    struct s_malloc_pointer_header* bucket_64;
    struct s_malloc_pointer_header* bucket_128;
    struct s_malloc_pointer_header* bucket_256;
    struct s_malloc_pointer_header* bucket_512;
} global_allocator = { 0 };

// Pages bottom
extern page_t pages_bottom;
page_t* pages_start = &pages_bottom;

enum {
    PAGE_ALLOC_BYTE_FREE = 0,
    PAGE_ALLOC_BYTE_USED = 1,
    PAGE_ALLOC_BYTE_LAST = 2,
};

// init_heap_metadata(void*) -> void
// Initialised the heap by allocating space for page metadata.
void init_heap_metadata(void* fdt) {
    fdt_t devicetree = verify_fdt(fdt);
    if (devicetree.header == (void*) 0) {
        console_puts("Failed to initialise heap: device tree header is invalid\n");
        return;
    }

    // TODO: check if the device tree is in the range
    unsigned int address_cells = be_to_le(32, fdt_get_property(&devicetree, (void*) 0, "#address-cells").data);
    unsigned int size_cells = be_to_le(32, fdt_get_property(&devicetree, (void*) 0, "#size-cells").data);
    struct fdt_property reg = fdt_get_property(&devicetree, fdt_find(&devicetree, "memory", (void*) 0), "reg");

    HEAP_SIZE = 0;
    for (int i = 4 * address_cells; i < reg.len; i += 4 * (size_cells + address_cells)) {
        HEAP_SIZE += be_to_le(32 * size_cells, reg.data + i);
    }
    console_printf("Heap has %llx bytes of memory\n", HEAP_SIZE);

    unsigned long long page_count = HEAP_SIZE / PAGE_SIZE / PAGE_SIZE;
    pages_start += page_count;
    volatile unsigned long long* ptr = (unsigned long long*) &pages_bottom;

    for (; ptr < (unsigned long long*) pages_start; ptr++) {
        *ptr = 0;
    }

    console_puts("Initialised heap\n");
}

// is_free(page_t*) -> char
// Checks if a page is free. Returns true if free.
char is_free(page_t* ptr) {
    char* cp = ((char*) &pages_bottom) + (((unsigned long long) ptr) - (unsigned long long) pages_start) / PAGE_SIZE;
    return *cp == PAGE_ALLOC_BYTE_FREE;
}

// is_used(page_t*) -> char
// Checks if a page is used. Returns true if used.
char is_used(page_t* ptr) {
    char* cp = ((char*) &pages_bottom) + (((unsigned long long) ptr) - (unsigned long long) pages_start) / PAGE_SIZE;
    return *cp != PAGE_ALLOC_BYTE_FREE;
}

// is_last(page_t*) -> char
// Checks if a page is the last page in an allocation. Returns true if that is the case.
char is_last(page_t* ptr) {
    char* cp = ((char*) &pages_bottom) + (((unsigned long long) ptr) - (unsigned long long) pages_start) / PAGE_SIZE;
    return (*cp & PAGE_ALLOC_BYTE_LAST) != 0;
}

static void mark_pages_as_used_unchecked(void* ptr, unsigned long long page_count) {
    char* cp = ((char*) &pages_bottom) + (((unsigned long long) ptr) - (unsigned long long) pages_start) / PAGE_SIZE;
    char* end = cp + page_count;
    for (; cp < end; cp++) {
        *cp = PAGE_ALLOC_BYTE_USED;
    }
    *(cp - 1) |= PAGE_ALLOC_BYTE_LAST;
}

// mark_pages_as_used(void*, unsigned long long) -> void
// Marks the given pages as used.
void mark_pages_as_used(void* ptr, unsigned long long size) {
    ptr = (void*) ((unsigned long long) ptr & ~(PAGE_SIZE - 1));
    unsigned long long page_count = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    mark_pages_as_used_unchecked(ptr, page_count);
}

// alloc_page(unsigned long long) -> void*
// Returns a zeroed out pointer to consecutive pages in memory.
void* alloc_page(unsigned long long page_count) {
    if (page_count == 0)
        return (void*) 0;

    // Get mmu if available
    volatile unsigned long long mmu = 0;
    mmu_level_1_t* top = (void*) 0;
    asm volatile("csrr %0, satp" : "=r" (mmu));
    if (mmu & 0x8000000000000000) {
        top = (void*) ((mmu & 0x00000fffffffffff) << 12);
    }

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
                if (top != (void*) 0) {
                    // Add pages to mmu if necessary
                    for (page_t* p = ptr; p < end; p++) {
                        premap_mmu(top, p);
                    }

                    // Check if still free
                    for (page_t* p = ptr; p < end; p++) {
                        if (is_used(p)) {
                            free = 0;
                            break;
                        }
                    }

                    // Map page
                    if (free) {
                        for (page_t* p = ptr; p < end; p++) {
                            map_mmu(top, p, p, MMU_FLAG_READ | MMU_FLAG_WRITE);
                        }
                    } else continue;
                }

                // Clear pages
                volatile unsigned long long* big_ptr = (unsigned long long*) ptr;
                for (; big_ptr < (unsigned long long*) ((void*) end); big_ptr++) {
                    *big_ptr = 0;
                }

                // Mark pages as used
                mark_pages_as_used_unchecked(ptr, page_count);

                return (void*) ptr;
            }
        }
    }

    // No pointer was found; return null
    console_printf("[alloc_page] Error: Could not allocate %llx consecutive pages!\n", page_count);
    return (void*) 0;
}

// dealloc_page(void*) -> void
// Deallocates a pointer allocated by alloc.
void dealloc_page(void* ptr) {
    if (ptr == (void*) 0)
        return;

    page_t* page_ptr = (page_t*) ptr;
    char* cp = ((char*) &pages_bottom) + (((unsigned long long) page_ptr) - (unsigned long long) pages_start) / PAGE_SIZE;

    while (!is_last(page_ptr)) {
        // Mark pages as free
        *cp = PAGE_ALLOC_BYTE_FREE;
        page_ptr++;
        cp++;
    }

    // Mark last page as free
    *cp = PAGE_ALLOC_BYTE_FREE;
}

struct s_malloc_pointer_header* memory_format_new_page(unsigned long int size) {
    void* page = alloc_page(1);
    if (page == (void*) 0)
        return page;
    unsigned long long node_size = (size + sizeof(struct s_malloc_pointer_header) + 15) & ~15;
    struct s_malloc_pointer_header* header = (void*) 0;
    for (unsigned long long i = 0; i < PAGE_SIZE; i += node_size) {
        if (header) {
            header->next = page + i;
            header = header->next;
        } else header = page + i;
        header->size = size;
    }

    return page;
}

#define MALLOC_GET_FROM_BUCKET(size)                                                                \
do {                                                                                                \
    if (n <= size) {                                                                                \
        if (!global_allocator.bucket_##size) {                                                      \
            global_allocator.bucket_##size = memory_format_new_page(size);                          \
                                                                                                    \
            if (!global_allocator.bucket_##size) {                                                  \
                console_printf("[malloc] Out of memory! Attempted to allocate %lx bytes.\n", n);    \
                return (void*) 0;                                                                   \
            }                                                                                       \
        }                                                                                           \
                                                                                                    \
        struct s_malloc_pointer_header* header = global_allocator.bucket_##size;                    \
        global_allocator.bucket_##size = global_allocator.bucket_##size->next;                      \
        return header + 1;                                                                          \
    }                                                                                               \
} while (0)

// malloc(unsigned long int) -> void*
// Allocates a small piece of memory
void* malloc(unsigned long int n) {
    // Don't allocate zero sized memory
    if (n == 0) return (void*) 0;

    unsigned long long ra;
    asm volatile("mv %0, ra" : "=r" (ra));
    MALLOC_GET_FROM_BUCKET(16);
    MALLOC_GET_FROM_BUCKET(32);
    MALLOC_GET_FROM_BUCKET(64);
    MALLOC_GET_FROM_BUCKET(128);
    MALLOC_GET_FROM_BUCKET(256);
    MALLOC_GET_FROM_BUCKET(512);
    unsigned long long page_count = (n + sizeof(struct s_malloc_pointer_header) + PAGE_SIZE - 1) / PAGE_SIZE;
    struct s_malloc_pointer_header* header = alloc_page(page_count);
    header->size = n;
    return header + 1;
}

#undef MALLOC_GET_FROM_BUCKET

// realloc(void*, unsigned long int) -> void*
// Reallocates a piece of memory, returning the new pointer.
void* realloc(void* ptr, unsigned long int n) {
    if (ptr == (void*) 0)
        return (void*) 0;

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
    switch (header->size) {
        case 16:
            header->next = global_allocator.bucket_16;
            global_allocator.bucket_16 = header;
            break;
        case 32:
            header->next = global_allocator.bucket_32;
            global_allocator.bucket_32 = header;
            break;
        case 64:
            header->next = global_allocator.bucket_64;
            global_allocator.bucket_64 = header;
            break;
        case 128:
            header->next = global_allocator.bucket_128;
            global_allocator.bucket_128 = header;
            break;
        case 256:
            header->next = global_allocator.bucket_256;
            global_allocator.bucket_256 = header;
            break;
        case 512:
            header->next = global_allocator.bucket_512;
            global_allocator.bucket_512 = header;
            break;
        default:
            if (header->size > 512) {
                dealloc_page(header);
            } else {
                console_printf("[free] Warning: attempted to free memory that likely was not allocated by malloc: %p\n", ptr);
            }
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
