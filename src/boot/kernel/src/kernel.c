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

void kinit() {
    console_puts("Initialising kernel\n");

    // Initialise heap
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

    // MMU is definitely enabled here, so get page table and mark all nonmeta entries as global
    unsigned long long mmu;
    mmu_level_1_t* top = (void*) 0;
    asm volatile("csrr %0, satp" : "=r" (mmu));
    top = (void*) ((mmu & 0x00000fffffffffff) << 12);
    make_all_global(top);

    // Load /sbin/init
    elf_t init = load_executable_elf_from_file(root, "/sbin/init");
    pid_t initd = load_elf_as_process(0, &init, 1);
    free_elf(&init);

    // Load the new page table and clean up the old page table
    mmu_level_1_t* new_top = fetch_process(initd)->mmu_data;
    copy_mmu_globals(new_top, top);
    mmu = 0x8000000000000000 | (((unsigned long long) new_top) >> 12);
    asm volatile("csrw satp, %0" : "=r" (mmu));
    clean_mmu_mappings(top, 0);

    // Jump to init process
    console_puts("Loaded initd.\n");
    jump_to_process(initd);
}

