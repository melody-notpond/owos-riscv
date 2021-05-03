#include "ext2.h"
#include "../memory.h"
#include "../virtio/block.h"
#include "../uart.h"

ext2fs_superblock_t* ext2_load_superblock() {
    unsigned long long page_num = (sizeof(ext2fs_superblock_t) + PAGE_SIZE - 1) / PAGE_SIZE;
    ext2fs_superblock_t* superblock = alloc(page_num);

    // TODO: don't hardcode this
    volatile unsigned char status;
    virtio_block_read(7, 2, superblock, (sizeof(ext2fs_superblock_t) + SECTOR_SIZE - 1) / SECTOR_SIZE, &status);
    while (status == 0xff);

    if (superblock->magic != 0xef53) {
        uart_puts("ext2 filesystem not found.\n");
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

ext2fs_block_descriptor_t* ext2_load_block_descriptor_table(ext2fs_superblock_t* superblock) {
    unsigned int table_size = superblock->blocks_count / superblock->blocks_per_group * sizeof(ext2fs_block_descriptor_t);
    unsigned int sector_count = (table_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
    unsigned int page_num = (table_size + PAGE_SIZE - 1) / PAGE_SIZE;
    ext2fs_block_descriptor_t* block_descriptor_table = alloc(page_num);

    // TODO: don't hardcode this
    volatile unsigned char status = 0;
    virtio_block_read(7, 4, block_descriptor_table, sector_count, &status);
    while (status == 0xff);

    return block_descriptor_table;
}

