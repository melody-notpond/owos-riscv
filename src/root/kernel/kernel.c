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
    uart_printf("Heap has 0x%llx bytes free.\n", memfree());

    // Mount root file system
    struct s_dir_entry entry = generic_dir_lookup(root, "/dev/virt-blk7");
    if (!mount_block_device(root, entry.value.block)) {
        uart_printf("Mounted root file system (/dev/%s)\n", entry.name);
    } else {
        uart_puts("Failed to mount file system\n");
        while (1);
    }

    // Get test file
    struct s_dir_entry owo = generic_dir_lookup(root, "/uwu/nya/owo");
    if (owo.tag != 0) {
        generic_file_t* file = owo.value.file;
        int c;
        while ((c = generic_file_read_char(file)) != EOF) {
            uart_putc(c);
        }
        uart_putc('\n');
        close_generic_file(file);
    } else {
        uart_puts("Could not find file /uwu/nya/owo\n");
    }

    cleanup_directory(root);
    unmount_generic_dir(root);
    clean_virtio_block_devices();

    uart_printf("Heap has 0x%llx bytes free.\n", memfree());

    // Hang
    while (1) {
        uart_getc();
    }
}

void kinit() {
    uart_puts("Initialising kernel\n");

    // Initialise heap
    init_heap_metadata();
    uart_printf("Heap has 0x%llx bytes.\n", memsize());

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

    uart_printf("uwu\nHeap has 0x%llx bytes.\n", memfree());

    // Probe for available virtio devices
    virtio_probe(dev);

    uart_printf("Heap has 0x%llx bytes.\n", memfree());

    // Register file systems
    register_fs_mounter(ext2_mount);

    // Jump to interrupt init code
    asm("j interrupt_init");
}

