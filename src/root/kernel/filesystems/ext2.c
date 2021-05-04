#include "ext2.h"
#include "../memory.h"
#include "../uart.h"

ext2fs_superblock_t* ext2_load_superblock(generic_block_t* block) {
    unsigned long long page_num = (sizeof(ext2fs_superblock_t) + PAGE_SIZE - 1) / PAGE_SIZE;
    ext2fs_superblock_t* superblock = alloc(page_num);

    generic_block_read(block, superblock, 2, (sizeof(ext2fs_superblock_t) + SECTOR_SIZE - 1) / SECTOR_SIZE);

    if (superblock->magic != 0xef53) {
        uart_puts("ext2 filesystem not found.\n");
        dealloc(superblock);
        return (ext2fs_superblock_t*) 0;
    }

    uart_puts("File system has 0x");
    uart_put_hex(superblock->inodes_count);
    uart_puts(" inodes.\n");

    uart_puts("File system has 0x");
    uart_put_hex(superblock->blocks_count);
    uart_puts(" blocks.\n");

    return superblock;
}

void* ext2fs_load_block(generic_block_t* block, ext2fs_superblock_t* superblock, unsigned int block_id) {
    unsigned long long block_size = 1024 << superblock->log_block_size;
    void* data = alloc((block_size + PAGE_SIZE - 1) / PAGE_SIZE);
    unsigned long long sector = block_id * block_size / SECTOR_SIZE;
    unsigned long long sector_count = (block_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
    generic_block_read(block, data, sector, sector_count);
    return data;
}

ext2fs_block_descriptor_t* ext2_load_block_descriptor_table(generic_block_t* block, ext2fs_superblock_t* superblock) {
    unsigned int table_size = superblock->blocks_count / superblock->blocks_per_group * sizeof(ext2fs_block_descriptor_t);
    unsigned int sector_count = (table_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
    unsigned int page_num = (table_size + PAGE_SIZE - 1) / PAGE_SIZE;
    ext2fs_block_descriptor_t* block_descriptor_table = alloc(page_num);

    generic_block_read(block, block_descriptor_table, (1024 << superblock->log_block_size) / SECTOR_SIZE, sector_count);

    return block_descriptor_table;
}

ext2fs_inode_t* ext2_get_root_inode(generic_block_t* block, ext2fs_superblock_t* superblock, ext2fs_block_descriptor_t* desc_table) {
    ext2fs_inode_t* root = alloc((superblock->inode_size + PAGE_SIZE - 1) / PAGE_SIZE);
    void* buffer = ext2fs_load_block(block, superblock, desc_table[0].inode_table);

    memcpy(root, buffer + superblock->inode_size, sizeof(ext2fs_inode_t));

    dealloc(buffer);
    return root;
}

