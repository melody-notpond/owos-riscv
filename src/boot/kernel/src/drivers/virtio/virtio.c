#include "block.h"
#include "gpu.h"
#include "../console/console.h"
#include "virtio.h"

#define VIRTIO_MAGIC 0x74726976

void virtio_probe(generic_dir_t* dev) {
    volatile void* base = (void*) VIRTIO_MMIO_BASE;

    for (; (long long) base <= VIRTIO_MMIO_TOP; base += VIRTIO_MMIO_INTERVAL) {
        console_printf("Probing %p for virtio devices.\n", base);

        volatile virtio_mmio_t* mmio = (volatile virtio_mmio_t*) base;
        if (mmio->magic_value != VIRTIO_MAGIC) {
            console_puts("Device is not a virtio device. Resuming probing.\n");
            continue;
        }

        if (mmio->version != 0x2) {
            console_puts("Legacy version of virtio is unsupported. Resuming probing.\n");
            continue;
        }

        switch (mmio->device_id) {
            case 0x00:
                console_puts("Device is unallocated. Resuming probing.\n");
                break;
            case 0x02:
                console_puts("Device is a block device. Initialising...\n");
                if (virtio_init_block_device(mmio)) {
                    console_puts("Failed to initialise block device\n");
                } else {
                    console_puts("Block device initialised successfully!\n");
                    virtio_block_make_generic(((unsigned long long) base - VIRTIO_MMIO_BASE) / VIRTIO_MMIO_INTERVAL, dev);
                }
                break;
            case 0x10:
                console_puts("Device is a graphics device. Initialising...\n");
                if (virtio_init_graphics_device(mmio))
                    console_puts("Failed to initialise graphics device\n");
                else
                    console_puts("Graphics device initialised successfully!\n");
                break;
            default:
                console_printf("Unknown device 0x%x. Resuming probing.\n", mmio->device_id);
                break;
        }
    }
}

