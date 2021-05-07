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

    // Mount root file system
    ext2fs_mount_t mount = ext2_mount(root_block);
    if (mount.root_inode != (void*) 0) {
        uart_puts("Mounted root file system (/dev/");
        uart_puts(mount.block->name);
        uart_puts(")\n");
    } else {
        uart_puts("Failed to mount file system\n");
        while (1);
    }


    // Get test file
    unsigned long long block_size = 1024 << mount.superblock->log_block_size;
    char* path[] = {"uwu", "nya", "owo"};
    ext2fs_inode_t* inode = ext2_get_inode(&mount, mount.root_inode, path, 3);

    if (inode != (void*) 0) {
        uart_puts("File found!\nInode entries:\n");
        for (int i = 0; i < 15; i++) {
            uart_put_hex(inode->block[i]);
            uart_putc('\n');
        }

        void* data = malloc(block_size);
        ext2_dump_inode_buffer(&mount, inode, data, 0);
        uart_put_hexdump(data, 32);
    } else {
        uart_puts("File not found.\n");
    }

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

