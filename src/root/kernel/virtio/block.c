#include "block.h"
#include "../memory.h"
#include "../uart.h"

// Devices
virtio_block_device_t block_devices[VIRTIO_DEVICE_COUNT] = { { 0 } };

char virtio_init_block_device(volatile virtio_mmio_t* mmio) {
    char ro;
    VIRTIO_GENERIC_INIT(mmio,
        ro = (features & (1 << 5)) != 0;
    ,
        // Check queue size
        unsigned int queue_num_max = mmio->queue_num_max;

        // If the queue size is too small, give up
        if (queue_num_max < VIRTIO_RING_SIZE) {
            uart_puts("Queue size is invalid.");
            return 0;
        }

        // Set queue size
        mmio->queue_num = VIRTIO_RING_SIZE;

        // Create block device queue
        unsigned long long page_count = (sizeof(virtio_queue_t) + PAGE_SIZE - 1) / PAGE_SIZE;
        uart_puts("Queue for block device has 0x");
        uart_put_hex(page_count);
        uart_puts(" pages.\n");

        // Select queue
        mmio->queue_sel = 0;

        // Allocate queue
        volatile virtio_queue_t* queue = alloc(page_count);

        // Notify device of queue
        mmio->queue_desc = (volatile virtio_descriptor_t**) &queue->desc;
        mmio->queue_avail = &queue->available;
        mmio->queue_used = &queue->used;

        // Get config
        volatile virtio_block_config_t* config = (volatile virtio_block_config_t*) (&mmio->config);
        uart_puts("Block device has 0x");
        uart_put_hex(config->capacity);
        uart_puts(" sectors.\n");

        // Add block device
        long long i = (((long long) mmio) - VIRTIO_MMIO_BASE) >> 12;
        block_devices[i].queue = queue;
        block_devices[i].mmio = mmio;
        block_devices[i].config = config;
        block_devices[i].idx = 0;
        block_devices[i].ack_used_idx = 0;
        block_devices[i].in_use = 1;
        block_devices[i].ro = ro;
    );

    return 1;
}

