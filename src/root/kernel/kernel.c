#include "filesystems/ext2.h"
#include "filesystems/generic_file.h"
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
    char* path[] = {"uwu", "nya", "owo"};
    unsigned int inode = ext2_get_inode(&mount, mount.root_inode, path, 3);

    if (inode != 0) {
        uart_puts("Found file /uwu/nya/owo\nContents of file:\n");
        generic_filesystem_t fs = ext2_create_generic_filesystem(&mount);
        generic_file_t file = ext2_create_generic_regular_file(&fs, inode);

        int c;
        while ((c = generic_file_read_char(&file)) != EOF) {
            uart_putc(c);
        }
        uart_putc('\n');
    } else {
        uart_puts("File /uwu/nya/owo not found.\n");
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

