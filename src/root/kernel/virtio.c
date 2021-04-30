#include "uart.h"
#include "virtio.h"

#define VIRTIO_MMIO_BASE 0x10001000
#define VIRTIO_MMIO_INTERVAL 0x1000
#define VIRTIO_MMIO_TOP 0x010008000
#define VIRTIO_MAGIC 0x74726976

#define VIRTIO_GENERIC_INIT(addr, device_features, device_specific)                                      \
    do {                                                                                                 \
        /* 1. Reset the device (set status register to zero) */                                          \
        volatile unsigned int* status = addr + 0x070;                                                    \
        *(status) = 0;                                                                                   \
                                                                                                         \
        /* 2. Acknowledge the device */                                                                  \
        *(status) |= VIRTIO_DEVICE_STATUS_ACKNOWLEDGE;                                                   \
                                                                                                         \
        /* 3. State the OS knows how to use the device */                                                \
        *(status) |= VIRTIO_DEVICE_STATUS_DRIVER;                                                        \
                                                                                                         \
        /* 4. Read device specific features */                                                           \
        unsigned int features = *((unsigned int*) addr + 0x010);                                         \
        { device_features }                                                                              \
        *((unsigned int*) addr + 0x020) = features;                                                      \
                                                                                                         \
        /* 5. Set features ok bit (can no longer set features) */                                        \
        *(status) |= VIRTIO_DEVICE_STATUS_FEATURES_OK;                                                   \
                                                                                                         \
        /* 6. Check that the features ok bit is still enabled */                                         \
        if (*(status) & VIRTIO_DEVICE_STATUS_FEATURES_OK == 0) {                                         \
            uart_puts("Virtio device does not support given features. Giving up on initialisation.\n");  \
            return;                                                                                      \
        }                                                                                                \
                                                                                                         \
        /* 7. Device specific initialisation */                                                          \
        { device_specific };                                                                             \
                                                                                                         \
        /* 8. Set driver ok bit, device is ready for use. */                                             \
        *(status) |= VIRTIO_DEVICE_STATUS_DRIVER_OK;                                                     \
    } while (0)

enum {
    VIRTIO_DEVICE_STATUS_ACKNOWLEDGE = 1,
    VIRTIO_DEVICE_STATUS_DRIVER = 2,
    VIRTIO_DEVICE_STATUS_FEATURES_OK = 8,
    VIRTIO_DEVICE_STATUS_DRIVER_OK = 4,
};

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
void virtio_init_block_device(volatile void* addr) {
    VIRTIO_GENERIC_INIT(status,
        features &= 0xffffffdf;
    ,
        
    );
}

void virtio_probe() {
    volatile void* base = (void*) VIRTIO_MMIO_BASE;

    for (; (long long) base <= VIRTIO_MMIO_TOP; base += VIRTIO_MMIO_INTERVAL) {
        uart_puts("Probing 0x");
        uart_put_hex((long long) base);
        uart_puts(" for virtio devices\n");

        if (*((unsigned int*) base) != VIRTIO_MAGIC) {
            uart_puts("Device is not a virtio device. Resuming probing.\n");
            continue;
        }

        switch (*(((unsigned int*) base) + 2)) {
            case 0x00:
                uart_puts("Device is unallocated. Resuming probing.\n");
                break;
            case 0x02:
                uart_puts("Device is a block device. Initialising...\n");
                virtio_init_block_device(base);
                break;
            default:
                uart_puts("Unknown device. Resuming probing.\n");
                break;
        }
    }
}
