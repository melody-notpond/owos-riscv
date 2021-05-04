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

ext2fs_block_descriptor_t* ext2_load_block_descriptor_table(ext2fs_superblock_t* superblock) {
    unsigned int table_size = superblock->blocks_count / superblock->blocks_per_group * sizeof(ext2fs_block_descriptor_t);
    unsigned int sector_count = (table_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
    unsigned int page_num = (table_size + PAGE_SIZE - 1) / PAGE_SIZE;
    ext2fs_block_descriptor_t* block_descriptor_table = alloc(page_num);

    // TODO: don't hardcode this
    volatile unsigned char status = 0;
    virtio_block_read(7, (1024 << superblock->log_block_size) / SECTOR_SIZE, block_descriptor_table, sector_count, &status);
    while (status == 0xff);

    return block_descriptor_table;
}

/*
unsigned char* ext2fs_fetch_blocks(ext2fs_superblock_t* superblock, unsigned int count, unsigned int block_ids[]) {
    unsigned long long block_size = 1024 << superblock->log_block_size;
    unsigned long long size = block_size * count;
    unsigned long long page_num = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    unsigned char* data = alloc(page_num);

    for (unsigned int i = 0; i < count; i++) {
        unsigned int block_id = block_ids[i];
        unsigned int sector = 4 * SECTOR_SIZE + sizeof(ext2fs_superblock_t) + sizeof(ext2fs_block_descriptor_t) * superblock->blocks_count / superblock->blocks_per_group + block_id * block_size + SECTOR_SIZE - 1;
        sector /= SECTOR_SIZE;

        // TODO: don't hardcode this
        unsigned char status;
        virtio_block_read(7, sector, data + i * block_size, size / SECTOR_SIZE, &status);
        while (status == 0xff);
    }

    return data;
}

void* ext2fs_fetch_consecutive_blocks(ext2fs_superblock_t* superblock, unsigned int block_id, unsigned int count) {
    unsigned long long block_size = 1024 << superblock->log_block_size;
    unsigned long long page_num = (block_size * count + PAGE_SIZE - 1) / PAGE_SIZE;
    unsigned char* data = alloc(page_num);
    unsigned long long size = block_size / SECTOR_SIZE;

    unsigned int sector = block_id * block_size;
    for (unsigned int i = 0; i < count; i++) {
        // TODO: don't hardcode this
        unsigned char status;
        virtio_block_read(7, sector / SECTOR_SIZE, data + i * block_size, size, &status);
        while (status == 0xff);
        sector += block_size;
    }

    return data;
}
*/

ext2fs_inode_t* ext2_get_root_inode(ext2fs_superblock_t* superblock, ext2fs_block_descriptor_t* desc_table) {
    unsigned long long block_size = 1024 << superblock->log_block_size;
    ext2fs_inode_t* root = alloc((superblock->inode_size + PAGE_SIZE - 1) / PAGE_SIZE);
    void* buffer = alloc((block_size + PAGE_SIZE - 1) / PAGE_SIZE);
    unsigned long long sector = desc_table[0].inode_table * block_size / SECTOR_SIZE;
    unsigned long long sector_count = (block_size + SECTOR_SIZE - 1) / SECTOR_SIZE;

    // TODO: don't hardcode this
    unsigned char status;
    virtio_block_read(7, sector, buffer, sector_count, &status);
    while (status == 0xff);

    memcpy(root, buffer + superblock->inode_size, sizeof(ext2fs_inode_t));

    dealloc(buffer);
    return root;
}

