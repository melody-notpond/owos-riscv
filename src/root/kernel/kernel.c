#include "filesystems/ext2.h"
#include "filesystems/generic_file.h"
#include "memory.h"
#include "string.h"
#include "uart.h"
#include "virtio/block.h"
#include "virtio/virtio.h"
#include "generic_block.h"

generic_dir_t* root;

void kmain() {
    uart_puts("Finished initialisation.\n");

    // Mount root file system
    generic_dir_t* dev = generic_dir_lookup_dir(root, "dev")->value.dir;
    struct s_dir_entry* entry = generic_dir_lookup_dir(dev, "virt-blk7");
    if (!mount_block_device(root, &entry->value.block)) {
        uart_printf("Mounted root file system (/dev/%s)\n", entry->name);
    } else {
        uart_puts("Failed to mount file system\n");
        while (1);
    }

    // Get test file
    char* path[] = {"uwu", "nya", "owo"};
    ext2fs_mount_t* mount = (*root)->fs.mount;
    unsigned int inode = ext2_get_inode(mount, mount->root_inode, path, 3);

    if (inode != 0) {
        uart_puts("Found file /uwu/nya/owo\nContents of file:\n");
        generic_file_t file = ext2_create_generic_regular_file(&(*root)->fs, inode);

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

    // Initialise root and /dev file system
    root = init_generic_dir();
    generic_dir_t* dev = init_generic_dir();

    // Add the /dev file system to the root file system
    generic_dir_append_entry(root, (struct s_dir_entry) {
        .name = strdup("dev"),
        .tag = DIR_ENTRY_TYPE_DIR,
        .value = {
            .dir = dev
        }
    });

    // Probe for available virtio devices
    virtio_probe(dev);

    // Register file systems
    register_fs_mounter(ext2_mount);

    // Jump to interrupt init code
    asm("j interrupt_init");
}

