#include "../memory.h"
#include "../uart.h"
#include "virtqueue.h"

// virtqueue_add_to_device(volatile virtio_mmio_t* mmio, unsigned int) -> volatile virtio_queue_t*
// Adds a virtqueue to a device.
volatile virtio_queue_t* virtqueue_add_to_device(volatile virtio_mmio_t* mmio, unsigned int queue_sel) {
    // Check queue size
    unsigned int queue_num_max = mmio->queue_num_max;

    // If the queue size is too small, give up
    if (queue_num_max < VIRTIO_RING_SIZE) {
        uart_puts("Queue size is invalid.");
        return (volatile virtio_queue_t*) 0;
    }

    // Select queue
    mmio->queue_sel = queue_sel;

    // Set queue size
    mmio->queue_num = VIRTIO_RING_SIZE;

    // Check that queue is not in use
    if (mmio->queue_ready) {
        uart_puts("Queue is being used.\n");
        return (volatile virtio_queue_t*) 0;
    }

    // Allocate queue
    volatile virtio_queue_t* queue = malloc(sizeof(virtio_queue_t) + VIRTIO_RING_SIZE * sizeof(virtio_descriptor_t) + sizeof(virtio_available_t) + sizeof(virtio_used_t));
    void* ptr = (void*) queue;
    queue->num = 0;
    queue->last_seen_used = 0;
    queue->desc = (ptr += sizeof(virtio_queue_t));
    queue->available = (ptr += VIRTIO_RING_SIZE * sizeof(virtio_descriptor_t));
    queue->available->flags = 0;
    queue->available->idx = 0;
    queue->used = (ptr += sizeof(virtio_available_t));
    queue->used->flags = 0;
    queue->used->idx = 0;

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
    return queue;
}

