.section .text
.global ext2fs_load_kernel
.global pci_init


/*
static const MemMapEntry virt_memmap[] = {
    [VIRT_DEBUG] =       {        0x0,         0x100 },
    [VIRT_MROM] =        {     0x1000,        0xf000 },
    [VIRT_TEST] =        {   0x100000,        0x1000 },
    [VIRT_RTC] =         {   0x101000,        0x1000 },
    [VIRT_CLINT] =       {  0x2000000,       0x10000 },
    [VIRT_PCIE_PIO] =    {  0x3000000,       0x10000 },
    [VIRT_PLIC] =        {  0xc000000, VIRT_PLIC_SIZE(VIRT_CPUS_MAX * 2) },
    [VIRT_UART0] =       { 0x10000000,         0x100 },
    [VIRT_VIRTIO] =      { 0x10001000,        0x1000 },
    [VIRT_FW_CFG] =      { 0x10100000,          0x18 },
    [VIRT_FLASH] =       { 0x20000000,     0x4000000 },
    [VIRT_PCIE_ECAM] =   { 0x30000000,    0x10000000 },
    [VIRT_PCIE_MMIO] =   { 0x40000000,    0x40000000 },
    [VIRT_DRAM] =        { 0x80000000,           0x0 },
};
(qemu) info pci
  Bus  0, device   0, function 0:
    Host bridge: PCI device 1b36:0008
      PCI subsystem 1af4:1100
      id ""
  Bus  0, device   1, function 0:
    SCSI controller: PCI device 1af4:1001
      PCI subsystem 1af4:0002
      IRQ 0, pin A
      BAR0: I/O at 0xffffffffffffffff [0x007e].
      BAR1: 32 bit memory at 0xffffffffffffffff [0x00000ffe].
      BAR4: 64 bit prefetchable memory at 0xffffffffffffffff [0x00003ffe].
      id ""

  dev: gpex-pcihost, id ""
    gpio-out "sysbus-irq" 4
    x-config-reg-migration-enabled = true
    mmio ffffffffffffffff/0000000020000000
    mmio ffffffffffffffff/ffffffffffffffff
    mmio 0000000003000000/0000000000010000
    bus: pcie.0
      type PCIE
      dev: virtio-blk-pci, id ""
        disable-legacy = "off"
        disable-modern = false
        class = 0 (0x0)
        ioeventfd = true
        vectors = 2 (0x2)
        virtio-pci-bus-master-bug-migration = false
        migrate-extra = true
        modern-pio-notify = false
        x-disable-pcie = false
        page-per-vq = false
        x-ignore-backend-features = false
        ats = false
        x-pcie-deverr-init = true
        x-pcie-lnkctl-init = true
        x-pcie-pm-init = true
        x-pcie-flr-init = true
        addr = 01.0
        romfile = ""
        rombar = 1 (0x1)
        multifunction = false
        x-pcie-lnksta-dllla = true
        x-pcie-extcap-init = true
        failover_pair_id = ""
        class SCSI controller, addr 00:01.0, pci id 1af4:1001 (sub 1af4:0002)
        bar 0: i/o at 0xffffffffffffffff [0x7e]
        bar 1: mem at 0xffffffffffffffff [0xffe]
        bar 4: mem at 0xffffffffffffffff [0x3ffe]
        bus: virtio-bus
          type virtio-pci-bus
          dev: virtio-blk-device, id ""
            drive = "mydrive"
            logical_block_size = 512 (512 B)
            physical_block_size = 512 (512 B)
            min_io_size = 0 (0 B)
            opt_io_size = 0 (0 B)
            discard_granularity = 4294967295 (4 GiB)
            write-cache = "auto"
            share-rw = false
            rerror = "auto"
            werror = "auto"
            cyls = 4161 (0x1041)
            heads = 16 (0x10)
            secs = 63 (0x3f)
            lcyls = 0 (0x0)
            lheads = 0 (0x0)
            lsecs = 0 (0x0)
            serial = ""
            config-wce = true
            scsi = false
            request-merging = true
            num-queues = 1 (0x1)
            queue-size = 256 (0x100)
            seg-max-adjust = true
            discard = true
            write-zeroes = true
            max-discard-sectors = 4194303 (0x3fffff)
            max-write-zeroes-sectors = 4194303 (0x3fffff)
            x-enable-wce-if-config-wce = true
            indirect_desc = true
            event_idx = true
            notify_on_empty = true
            any_layout = true
            iommu_platform = false
            packed = false
            use-started = true
            use-disabled-flag = true
            x-disable-legacy-check = false
      dev: gpex-root, id ""
        addr = 00.0
        romfile = ""
        rombar = 1 (0x1)
        multifunction = false
        x-pcie-lnksta-dllla = true
        x-pcie-extcap-init = true
        failover_pair_id = ""
        class Host bridge, addr 00:00.0, pci id 1b36:0008 (sub 1af4:1100)
*/

.set HARD_DRIVE_BASE, 0x10001000


# pci_init(void) -> void
# Initialises the PCI.
#
# Parameters: nothing
# Returns: nothing
pci_init:
    ret


# ext2fs_load_kernel(void) -> void
# Loads the kernel from an ext2 file system.
#
# Parameters: nothing
# Returns: nothing
ext2fs_load_kernel:
    # Push the return address
    addi sp, sp, -0x8
    sd ra, 0x0(sp)

    # Tell user that hard drive is being loaded and booted into
    la a0, accessing_msg
    jal loader_uart_puts

    # Init virtio device
    jal init_virtio_hd_device
    beqz a0, ext2fs_return

    # Seek to magic number
    li a0, 0x0438
    jal ext2fs_hd_seek

    # Read magic number
    jal ext2fs_hd_read

    # Compare magic number
    bne t0, t1, ext2fs_return

    # Print message saying the file system magic number was verified
    la a0, ext2_magic_verified
    jal loader_uart_puts

    # Return
ext2fs_return:
    ld ra, 0x0(sp)
    addi sp, sp, 0x8
    ret


# init_virtio_hd_device(void) -> bool
# Initialises the virtio hard drive device.
#
# Parameters: nothing
# Returns:
# a0: bool      - True if successful, false if an error occured in initialisation
init_virtio_hd_device:
    addi sp, sp, -0x8
    sd ra, 0x0(sp)

    li t0, HARD_DRIVE_BASE
    lw t1, 0x00(t0)
    li t2, 0x74726976
    beq t1, t2, init_virtio_hd_device_magic_confirmed

    la a0, virtio_hd_device_bad_magic
    jal loader_uart_puts
    mv a0, zero
    j init_virtio_hd_device_return

init_virtio_hd_device_magic_confirmed:
    li a0, 1

init_virtio_hd_device_return:
    ld ra, 0x0(sp)
    addi sp, sp, 0x8
    ret


# ext2fs_hd_seek(long long) -> void
# Seeks to a memory address in the hard drive.
#
# Parameters:
# a0: long long - The address to seek to.
# Returns: nothing
ext2fs_hd_seek:
    ret


ext2fs_hd_read:
    ret


.section .rodata
accessing_msg:
    .string "Accessing hard drive...\n"
    .byte 0

ext2_magic_verified:
    .string "ext2 file system verified.\n"
    .byte 0

virtio_hd_device_bad_magic:
    .string "Magic number check for virtio hard drive device failed.\n"
    .byte 0
