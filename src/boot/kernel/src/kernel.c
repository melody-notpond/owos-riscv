#include "drivers/filesystems/ext2.h"
#include "drivers/filesystems/generic_file.h"
#include "lib/memory.h"
#include "userspace/elffile.h"
#include "userspace/process.h"
#include "lib/string.h"
#include "drivers/uart/uart.h"
#include "drivers/virtio/block.h"
#include "drivers/virtio/virtio.h"
#include "drivers/generic_block.h"

#define ROOT_DISC "/dev/virt-blk7"

generic_dir_t* root;
char running = 1;

void kmain() {
    uart_puts("Finished initialisation.\n");
    uart_printf("Heap has 0x%llx bytes free.\n", memfree());

    // Mount root file system
    struct s_dir_entry entry = generic_dir_lookup(root, ROOT_DISC);
    if (!mount_block_device(root, entry.value.block)) {
        uart_puts("Mounted root file system (" ROOT_DISC ")\n");
    } else {
        uart_puts("Failed to mount file system\n");
        while (1);
    }

    // Get /etc/fstab
    struct s_dir_entry fstab = generic_dir_lookup(root, "/etc/fstab");
    if (fstab.tag == DIR_ENTRY_TYPE_REGULAR) {
        // Put the contents of fstab on the UART port
        // TODO: do fstab-y things
        uart_puts("Found /etc/fstab:\n");
        generic_file_t* file = fstab.value.file;
        int c;
        while ((c = generic_file_read_char(file)) != EOF) {
            uart_putc(c);
        }
        uart_putc('\n');
        close_generic_file(file);
    } else {
        if (fstab.tag == DIR_ENTRY_TYPE_DIR)
            cleanup_directory(fstab.value.dir);
        uart_puts("Could not find file /etc/fstab\n");
    }

    // Load /bin/simple
    elf_t simple = load_executable_elf_from_file(root, "/bin/simple");
    load_elf_as_process(0, &simple, 1);
    free_elf(&simple);

    // Hang
    while (running) {
        uart_getc();
    }

    cleanup_directory(root);
    unmount_generic_dir(root);
    clean_virtio_block_devices();

    uart_printf("Heap has 0x%llx bytes free.\n", memfree());
}

void kinit() {
    uart_puts("Initialising kernel\n");

    // Initialise heap
    init_heap_metadata();
    uart_printf("Heap has 0x%llx bytes.\n", memsize());

    // Initialise process table
    init_process_table();

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

