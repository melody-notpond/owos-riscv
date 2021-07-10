#include "drivers/filesystems/ext2.h"
#include "drivers/filesystems/generic_file.h"
#include "drivers/generic_block.h"
#include "drivers/console/console.h"
#include "drivers/virtio/block.h"
#include "drivers/virtio/virtio.h"
#include "kshell.h"
#include "lib/memory.h"
#include "lib/string.h"
#include "userspace/elffile.h"
#include "userspace/process.h"
#include "userspace/mmu.h"

#define ROOT_DISC "/dev/virt-blk7"

generic_dir_t* root;
char running = 1;

mmu_level_1_t* kinit() {
    console_puts("Initialising kernel\n");

    // Initialise heap
    init_heap_metadata();
    console_printf("Heap has 0x%llx bytes.\n", memsize());

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

    // Create mmu page table
    mmu_level_1_t* top = create_mmu_top();
    mmu_map_kernel(top);
    return top;
}

void kmain() {
    console_puts("Finished initialisation.\n");
    console_printf("Heap has 0x%llx bytes free.\n", memfree());

    // Mount root file system
    struct s_dir_entry entry = generic_dir_lookup(root, ROOT_DISC);
    if (!mount_block_device(root, entry.value.block)) {
        console_puts("Mounted root file system (" ROOT_DISC ")\n");
    } else {
        console_puts("Failed to mount file system\n");
        while (1);
    }

    // Get /etc/fstab
    /*
    struct s_dir_entry fstab = generic_dir_lookup(root, "/etc/fstab");
    if (fstab.tag == DIR_ENTRY_TYPE_REGULAR) {
        // Put the contents of fstab on the UART port
        // TODO: do fstab-y things
        console_puts("Found /etc/fstab:\n");
        generic_file_t* file = fstab.value.file;
        int c;
        while ((c = generic_file_read_char(file)) != EOF) {
            console_putc(c);
        }
        console_putc('\n');
        close_generic_file(file);
    } else {
        if (fstab.tag == DIR_ENTRY_TYPE_DIR)
            cleanup_directory(fstab.value.dir);
        console_puts("Could not find file /etc/fstab\n");
    }
    */

    // Load /bin/simple
    elf_t simple = load_executable_elf_from_file(root, "/bin/simple");
    pid_t simpled = load_elf_as_process(0, &simple, 1);
    free_elf(&simple);

    // Jump to simple process
    console_puts("Loaded simpled.\n");
    jump_to_process(simpled);

    /*
    // Hang
    while (running) {
        console_getc();
    }

    kshell_main();

    cleanup_directory(root);
    unmount_generic_dir(root);
    clean_virtio_block_devices();

    console_printf("Heap has 0x%llx bytes free.\n", memfree());
    */
}

