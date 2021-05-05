#include "ext2.h"
#include "../memory.h"
#include "../uart.h"

// ext2_load_superblock(generic_block_t*) -> ext2fs_superblock_t*
// Loads a superblock from a block device.
ext2fs_superblock_t* ext2_load_superblock(generic_block_t* block) {
    // Allocate the superblock in memory
    ext2fs_superblock_t* superblock = malloc(sizeof(ext2fs_superblock_t));

    // Read the superblock
    generic_block_read(block, superblock, 2, (sizeof(ext2fs_superblock_t) + SECTOR_SIZE - 1) / SECTOR_SIZE);

    // Check the magic number
    if (superblock->magic != 0xef53) {
        uart_puts("ext2 filesystem not found.\n");
        free(superblock);
        return (ext2fs_superblock_t*) 0;
    }

    // Debug info
    uart_puts("File system has 0x");
    uart_put_hex(superblock->inodes_count);
    uart_puts(" inodes.\n");

    uart_puts("File system has 0x");
    uart_put_hex(superblock->blocks_count);
    uart_puts(" blocks.\n");

    return superblock;
}

// ext2fs_load_block(generic_block_t*, ext2fs_superblock_t*, unsigned int) -> void*
// Loads a block from an ext2 file system.
void* ext2fs_load_block(generic_block_t* block, ext2fs_superblock_t* superblock, unsigned int block_id) {
    // Allocate the block in memory
    unsigned long long block_size = 1024 << superblock->log_block_size;
    void* data = malloc(block_size);

    // Read the block
    unsigned long long sector = block_id * block_size / SECTOR_SIZE;
    unsigned long long sector_count = block_size / SECTOR_SIZE;
    generic_block_read(block, data, sector, sector_count);

    return data;
}

// ext2_load_block_descriptor_table(generic_block_t*, ext2fs_superblock_t*) -> ext2fs_block_descriptor_t*
// Loads a descriptor table from an ext2 file system.
ext2fs_block_descriptor_t* ext2_load_block_descriptor_table(generic_block_t* block, ext2fs_superblock_t* superblock) {
    // Allocate the table
    unsigned int table_size = (superblock->blocks_count + superblock->blocks_per_group - 1) / superblock->blocks_per_group * sizeof(ext2fs_block_descriptor_t);
    ext2fs_block_descriptor_t* block_descriptor_table = malloc(table_size);

    // Read the table
    unsigned int sector_count = (table_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
    generic_block_read(block, block_descriptor_table, (1024 << superblock->log_block_size) / SECTOR_SIZE, sector_count);

    return block_descriptor_table;
}

// ext2_get_root_inode(generic_block_t*, ext2fs_superblock_t*, ext2fs_block_descriptor_t*) -> ext2fs_inode_t*
// Loads the root inode from an ext2 file system.
ext2fs_inode_t* ext2_get_root_inode(generic_block_t* block, ext2fs_superblock_t* superblock, ext2fs_block_descriptor_t* desc_table) {
    // Allocate the inode and load the block in memory
    ext2fs_inode_t* root = malloc(superblock->inode_size);
    void* buffer = ext2fs_load_block(block, superblock, desc_table[0].inode_table);

    // Copy
    memcpy(root, buffer + superblock->inode_size, sizeof(ext2fs_inode_t));

    // Deallocate the buffer
    free(buffer);
    return root;
}


