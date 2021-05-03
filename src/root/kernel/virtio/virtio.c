#include "block.h"
#include "gpu.h"
#include "../uart.h"
#include "virtio.h"

#define VIRTIO_MMIO_INTERVAL 0x1000
#define VIRTIO_MMIO_TOP 0x010008000
#define VIRTIO_MAGIC 0x74726976

void virtio_probe() {
    volatile void* base = (void*) VIRTIO_MMIO_BASE;

    for (; (long long) base <= VIRTIO_MMIO_TOP; base += VIRTIO_MMIO_INTERVAL) {
        uart_puts("Probing 0x");
        uart_put_hex((long long) base);
        uart_puts(" for virtio devices\n");

        volatile virtio_mmio_t* mmio = (volatile virtio_mmio_t*) base;
        if (mmio->magic_value != VIRTIO_MAGIC) {
            uart_puts("Device is not a virtio device. Resuming probing.\n");
            continue;
        }

        if (mmio->version != 0x2) {
            uart_puts("Legacy version of virtio is unsupported. Resuming probing.\n");
            continue;
        }

        switch (mmio->device_id) {
            case 0x00:
                uart_puts("Device is unallocated. Resuming probing.\n");
                break;
            case 0x02:
                uart_puts("Device is a block device. Initialising...\n");
                if (virtio_init_block_device(mmio))
                    uart_puts("Failed to initialise block device\n");
                else
                    uart_puts("Block device initialised successfully!\n");
                break;
            case 0x10:
                uart_puts("Device is a graphics device. Initialising...\n");
                if (virtio_init_graphics_device(mmio))
                    uart_puts("Failed to initialise graphics device\n");
                else
                    uart_puts("Graphics device initialised successfully!\n");
                break;
            default:
                uart_puts("Unknown device 0x");
                uart_put_hex(mmio->device_id);
                uart_puts(". Resuming probing.\n");
                break;
        }
    }
}
