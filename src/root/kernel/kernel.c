#include "filesystems/ext2.h"
#include "memory.h"
#include "uart.h"
#include "virtio/block.h"
#include "virtio/virtio.h"
#include "generic_block.h"

generic_block_t root_block[128] = { 0 };
generic_block_t* last_block = root_block;

void kmain() {
    uart_puts("Finished initialisation.\n");

    // Temporary block writing tests
    /*
    uart_puts("Writing test data to block.\n");
    generic_block_write(root_block, "hewwo uwu aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 0, 1);
    uart_puts("Wrote to block device.\n");
    uart_puts("Reading test data from block\n");
    char data[513];
    data[512] = 0;
    generic_block_read(root_block, data, 0, 1);
    uart_puts("Read data to memory: \n");
    uart_put_hexdump(data, 512);
    */

    // File system stuff
    ext2fs_superblock_t* superblock = ext2_load_superblock(root_block);
    ext2fs_block_descriptor_t* descriptor_table = ext2_load_block_descriptor_table(root_block, superblock);
    ext2fs_inode_t* root_inode = ext2_get_root_inode(root_block, superblock, descriptor_table);

    uart_puts("Root inode has 0x");
    uart_put_hex(root_inode->blocks);
    uart_puts(" entries.\nRoot inode entries:\n");
    for (int i = 0; i < 15; i++) {
        uart_put_hex(root_inode->block[i]);
        uart_putc('\n');
    }

    unsigned long long block_size = 1024 << superblock->log_block_size;
    void* data_2 = ext2fs_load_block(root_block, superblock, root_inode->block[0]);
    // uart_put_hexdump(data_2, block_size);

    // Hang
    while (1) {
        uart_getc();
    }
}

void kinit() {
    uart_puts("Initialising kernel\n");

    // Initialise heap
    init_heap_metadata();

    // Probe for available virtio devices
    virtio_probe(&last_block);

    // Jump to interrupt init code
    asm("j interrupt_init");
}

