#ifndef KERNEL_GENERIC_BLOCK_H
#define KERNEL_GENERIC_BLOCK_H

#define SECTOR_SIZE 512

// Represents a generic block device.
// The unpack functions are used as follows (same goes for write):
// block->unpack_read(&data_buffer, sector, sector_count, block->metadata)
// On success, unpack_(read/write) returns 0, otherwise it returns an error code.
typedef struct s_generic_block {
    char (*unpack_read)(void*, unsigned long long, unsigned long long, unsigned char*);
    char (*unpack_write)(void*, unsigned long long, unsigned long long, unsigned char*);
    struct s_generic_block* next;
    unsigned char metadata[16];
} __attribute__((__packed__, aligned(4))) generic_block_t;

static inline char generic_block_read(generic_block_t* block, void* buffer, unsigned long long sector, unsigned long long sector_count) {
    return block->unpack_read(buffer, sector, sector_count, block->metadata);
}

static inline char generic_block_write(generic_block_t* block, void* buffer, unsigned long long sector, unsigned long long sector_count) {
    return block->unpack_write(buffer, sector, sector_count, block->metadata);
}

#endif /* KERNEL_GENERIC_BLOCK_H */

