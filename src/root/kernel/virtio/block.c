#include "block.h"
#include "../memory.h"
#include "../uart.h"

// Devices
virtio_block_device_t block_devices[VIRTIO_DEVICE_COUNT] = { { 0 } };

char virtio_init_block_device(volatile virtio_mmio_t* mmio) {
    char ro;
    VIRTIO_GENERIC_INIT(mmio, ro = (features & (1 << 5)) != 0;);

    // Check queue size
    unsigned int queue_num_max = mmio->queue_num_max;

    // If the queue size is too small, give up
    if (queue_num_max < VIRTIO_RING_SIZE) {
        uart_puts("Queue size is invalid.");
        return -1;
    }

    // Select queue
    mmio->queue_sel = 0;

    // Set queue size
    mmio->queue_num = VIRTIO_RING_SIZE;

    // Create block device queue
    unsigned long long page_count = (sizeof(virtio_queue_t) + VIRTIO_RING_SIZE * sizeof(virtio_descriptor_t) + sizeof(virtio_available_t) + sizeof(virtio_used_t) + PAGE_SIZE - 1) / PAGE_SIZE;
    uart_puts("Queue for block device has 0x");
    uart_put_hex(page_count);
    uart_puts(" pages.\n");

    // Check that queue is not in use
    if (mmio->queue_ready) {
        uart_puts("Queue is being used somehow.\n");
        return -1;
    }

    // Allocate queue
    volatile virtio_queue_t* queue = alloc(page_count);
    void* ptr = (void*) queue;
    queue->num = 0;
    queue->desc = (ptr += sizeof(virtio_queue_t));
    queue->available = (ptr += VIRTIO_RING_SIZE * sizeof(virtio_descriptor_t));
    queue->used = (ptr += sizeof(virtio_available_t));

    // Notify device of queue
    mmio->queue_num = VIRTIO_RING_SIZE;

    // Give device queue addresses
    mmio->queue_desc_low = (unsigned int) (unsigned long long) queue->desc;
    mmio->queue_desc_high = ((unsigned int) (((unsigned long long) queue->desc) >> 32));
    mmio->queue_avail_low = (unsigned int) (unsigned long long) queue->available;
    mmio->queue_avail_high = ((unsigned int) (((unsigned long long) queue->available) >> 32));
    mmio->queue_used_low = (unsigned int) (unsigned long long) queue->used;
    mmio->queue_used_high = ((unsigned int) (((unsigned long long) queue->used) >> 32));

    // State that queue is ready
    mmio->queue_ready = 1;

    // Get config
    volatile virtio_block_config_t* config = (volatile virtio_block_config_t*) mmio->config;
    uart_puts("Block device has 0x");
    uart_put_hex(config->capacity);
    uart_puts(" sectors.\n");

    // Add block device
    long long i = (((long long) mmio) - VIRTIO_MMIO_BASE) >> 12;
    block_devices[i] = (virtio_block_device_t) {
        .queue = queue,
        .mmio = mmio,
        .config = config,
        .in_use = 1,
        .ro = ro
    };

    // Finish initialisation
    VIRTIO_GENERIC_INIT_FINISH(mmio);
    return 0;
}

virtio_block_error_code_t virtio_block_write(unsigned char block_id, unsigned long long sector, void* data, unsigned long long size) {
    if (block_id >= 8) {
        return VIRTIO_BLOCK_ERROR_CODE_INVALID_DEVICE;
    } else if (!block_devices[block_id].in_use) {
        return VIRTIO_BLOCK_ERROR_CODE_NOT_BLOCK_DEVICE;
    } else if (block_devices[block_id].ro) {
        return VIRTIO_BLOCK_ERROR_CODE_READ_ONLY;
    } else if (block_devices[block_id].config->capacity < sector) {
        return VIRTIO_BLOCK_ERROR_CODE_OPERATION_BEYOND_CAPACITY;
    }

    // Allocate request
    virtio_block_request_t* request = alloc(1);
    *request = (virtio_block_request_t) {
        .type = VIRTIO_BLOCK_REQUEST_TYPE_OUT,
        .sector = sector
    };
    unsigned char* status = (unsigned char*) (request + 1);
    *status = 0;

    virtio_block_device_t* device = &block_devices[block_id];

    device->queue->desc[device->queue->num].addr = request;
    device->queue->desc[device->queue->num].flags = VIRTIO_DESCRIPTOR_FLAG_NEXT;
    device->queue->desc[device->queue->num].length = sizeof(virtio_block_request_t);
    device->queue->desc[device->queue->num].next = device->queue->num + 1;
    device->queue->desc[device->queue->num + 1].addr = data;
    device->queue->desc[device->queue->num + 1].flags = VIRTIO_DESCRIPTOR_FLAG_NEXT;
    device->queue->desc[device->queue->num + 1].length = size;
    device->queue->desc[device->queue->num + 1].next = device->queue->num + 2;
    device->queue->desc[device->queue->num + 2].addr = status;
    device->queue->desc[device->queue->num + 2].flags = VIRTIO_DESCRIPTOR_FLAG_WRITE_ONLY;
    device->queue->desc[device->queue->num + 2].length = 1;
    device->queue->desc[device->queue->num + 2].next = 0;

    device->queue->available->ring[device->queue->available->idx] = device->queue->num;
    device->queue->num += 3;
    device->queue->available->idx++;

    device->mmio->queue_notify = 0;

    return VIRTIO_BLOCK_ERROR_CODE_SUCCESS;
}

