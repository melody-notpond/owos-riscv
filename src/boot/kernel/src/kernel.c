#include "drivers/filesystems/ext2.h"
#include "drivers/filesystems/generic_file.h"
#include "drivers/generic_block.h"
#include "drivers/console/console.h"
#include "drivers/devicetree/tree.h"
#include "drivers/virtio/block.h"
#include "drivers/virtio/virtio.h"
#include "interrupts.h"
#include "lib/memory.h"
#include "lib/string.h"
#include "opensbi.h"
#include "userspace/elffile.h"
#include "userspace/process.h"
#include "userspace/mmu.h"

#define ROOT_DISC "/dev/virt-blk7"

generic_file_t* root;
trap_t trap_structs[32];

void kinit(unsigned long long hartid, void* fdt) {
    fdt_header_t* fdt_header = verify_fdt(fdt);
    console_printf("Initialising kernel with hartid 0x%llx and device tree located at %p\n", hartid, fdt_header);

    // Init console file system
    console_fs = (generic_filesystem_t) {
        .read_char = console_generic_file_read,
        .write_char = console_generic_file_write
    };

    // Create trap structure
    trap_structs[hartid] = (trap_t) {
        .hartid = hartid,
        .pid = 0,
        .pc = 0,
        .xs = { 0 },
        .fs = { 0.0 }
    };
    trap_t* trap = &trap_structs[hartid];
    asm volatile("csrw sscratch, %0" : "=r" (trap));

    // Initialise heap
    console_printf("Heap has 0x%llx bytes.\n", memsize());

    // Initialise process table
    init_process_table();

    // Initialise root and /dev file system
    root = malloc(sizeof(generic_file_t));
    *root = (generic_file_t) {
        .type = GENERIC_FILE_TYPE_DIR,
        .parent = (void*) 0,
        .fs = (void*) 0,
        .dir = init_generic_dir()
    };
    generic_file_t* dev = malloc(sizeof(generic_file_t));
    *dev = (generic_file_t) {
        .type = GENERIC_FILE_TYPE_DIR,
        .fs = (void*) 0,
        .dir = init_generic_dir()
    };

    // Add the /dev file system to the root file system
    generic_dir_append_entry(root, (struct s_dir_entry) {
        .name = strdup("dev"),
        .file = dev
    });

    // Probe for available virtio devices
    virtio_probe(dev);

    // Register file systems
    register_fs_mounter(ext2_mount);

    // Enable all interrupts in the PLIC
    volatile unsigned int* enables = get_context_enable_bits(PLIC_CONTEXT(hartid, 1));
    for (unsigned int i = 0; i < (PLIC_COUNT + 1) / 32; i++) {
        enables[i] = 0xffffffff;
    }

    // Set priority threshold
    volatile unsigned int* threshold = get_context_priority_threshold(PLIC_CONTEXT(hartid, 1));
    *threshold = 0;

    // Set next time interrupt
    unsigned long long time = 0;
    asm volatile("csrr %0, time" : "=r" (time));

    // Enable interrupts in the hart
    unsigned long long t = 0x202;
    asm volatile("csrs sie, %0" : "=r" (t));
    t = 0x22;
    asm volatile("csrs sstatus, %0" : "=r" (t));
}

void kmain() {
    console_puts("Finished initialisation.\n");
    console_printf("Heap has 0x%llx bytes free.\n", memfree());

    // Mount root file system
    struct s_dir_entry entry = generic_dir_lookup(root, ROOT_DISC);
    if (!mount_block_device(root, entry.file->block)) {
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
            sbi_console_putchar(c);
        }
        sbi_console_putchar('\n');
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
    pid_t initd = load_elf_as_process(1, &init, 1);
    free_elf(&init);
    process_t* initd_process = fetch_process(initd);

    // Set up stdin, stdout, and stderr
    initd_process->file_descriptors[0] = malloc(sizeof(generic_file_t));
    *initd_process->file_descriptors[0] = (generic_file_t) {
        .type = GENERIC_FILE_TYPE_SPECIAL,
        .fs = &console_fs
    };
    initd_process->file_descriptors[1] = malloc(sizeof(generic_file_t));
    *initd_process->file_descriptors[1] = (generic_file_t) {
        .type = GENERIC_FILE_TYPE_SPECIAL,
        .fs = &console_fs
    };
    initd_process->file_descriptors[2] = malloc(sizeof(generic_file_t));
    *initd_process->file_descriptors[2] = (generic_file_t) {
        .type = GENERIC_FILE_TYPE_SPECIAL,
        .fs = &console_fs
    };

    // Load the new page table and clean up the old page table
    process_init_kernel_mmu(initd);
    mmu_level_1_t* new_top = initd_process->mmu_data;
    mmu = 0x8000000000000000 | (((unsigned long long) new_top) >> 12);
    asm volatile("csrw satp, %0" : "=r" (mmu));
    clean_mmu_mappings(top, 0);
    asm volatile("sfence.vma");

    // Queue init process
    add_process_to_queue(initd);
    console_puts("Loaded initd.\n");
    sbi_set_timer(0);
    unsigned long long t = 0x222;
    asm volatile("csrs sie, %0" : "=r" (t));
    while (1);
}

