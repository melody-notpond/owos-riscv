#include "block.h"
#include "../../interrupts.h"
#include "../../lib/memory.h"
#include "../../lib/string.h"
#include "../console/console.h"

typedef struct __attribute__((__packed__, aligned(1))) { 
    unsigned long long capacity; 
    unsigned int size_max; 
    unsigned int seg_max; 
    struct { 
        unsigned short cylinders; 
        unsigned char heads; 
        unsigned char sectors; 
    } geometry; 
    unsigned int blk_size; 
    struct { 
        unsigned char physical_block_exp; 
        unsigned char alignment_offset; 
        unsigned short min_io_size; 
        unsigned int opt_io_size; 
    } topology; 
    unsigned char writeback; 
} virtio_block_config_t;

typedef struct {
    volatile virtio_queue_t* queue;
    volatile virtio_mmio_t* mmio;
    volatile virtio_block_config_t* config;
    char in_use;
    char ro;
} virtio_block_device_t;

typedef struct __attribute__((__packed__, aligned(4))) {
    unsigned int type;
    unsigned char rsv1[4];
    unsigned long long sector;
} virtio_block_request_t;

// Devices
virtio_block_device_t block_devices[VIRTIO_DEVICE_COUNT] = { { 0 } };

// virtio_block_mei_handler(unsigned int) -> void
// Handles a machine external interrupt for a virtio block device.
void virtio_block_mei_handler(unsigned int mei_id) {
    // Acknowledge the interrupt
    volatile virtio_block_device_t* device = &block_devices[mei_id - 1];
    unsigned int status = device->mmio->interrupt_status;
    device->mmio->interrupt_ack = status;

    if (status == VIRTIO_INTERRUPT_USED_RING_UPDATE) {
        // Free memory that is no longer used
        volatile virtio_descriptor_t* p;
        while ((p = virtqueue_pop_used(device->queue))) {
            free((void*) p->addr);
        }
    }
}

char virtio_init_block_device(volatile virtio_mmio_t* mmio) {
    char ro;
    VIRTIO_GENERIC_INIT(mmio, ro = (features & (1 << 5)) != 0;);

    volatile virtio_queue_t* queue = virtqueue_add_to_device(mmio, 0);
    if (queue == 0)
        return -1;

    // Get config
    volatile virtio_block_config_t* config = (volatile virtio_block_config_t*) mmio->config;
    console_printf("Block device has 0x%llx sectors.\n", config->capacity);

    // Add block device
    long long i = (((long long) mmio) - VIRTIO_MMIO_BASE) / VIRTIO_MMIO_INTERVAL;
    block_devices[i] = (virtio_block_device_t) {
        .queue = queue,
        .mmio = mmio,
        .config = config,
        .in_use = 1,
        .ro = ro
    };

    // Add interrupt
    if (register_mei_handler(i + 1, 7, virtio_block_mei_handler)) {
        console_puts("Warning: interrupt handler for block device is unregistered\n");
    }

    // Finish initialisation
    VIRTIO_GENERIC_INIT_FINISH(mmio);
    return 0;
}

typedef enum {
    VIRTIO_BLOCK_OPERATION_READ = 0,
    VIRTIO_BLOCK_OPERATION_WRITE = 1
} virtio_block_operation_t;

enum {
    VIRTIO_BLOCK_REQUEST_TYPE_IN = 0,
    VIRTIO_BLOCK_REQUEST_TYPE_OUT = 1,
    VIRTIO_BLOCK_REQUEST_TYPE_FLUSH = 4,
};

virtio_block_error_code_t virtio_block_operation(virtio_block_operation_t rw, unsigned char block_id, unsigned long long sector, void* data, unsigned long long size, volatile unsigned char* status) {
    if (block_id >= 8) {
        return VIRTIO_BLOCK_ERROR_CODE_INVALID_DEVICE;
    } else if (!block_devices[block_id].in_use) {
        return VIRTIO_BLOCK_ERROR_CODE_NOT_BLOCK_DEVICE;
    } else if (rw == VIRTIO_BLOCK_OPERATION_WRITE && block_devices[block_id].ro) {
        return VIRTIO_BLOCK_ERROR_CODE_READ_ONLY;
    } else if (block_devices[block_id].config->capacity < sector) {
        return VIRTIO_BLOCK_ERROR_CODE_OPERATION_BEYOND_CAPACITY;
    }

    // Allocate request
    virtio_block_request_t* request = malloc(sizeof(virtio_block_request_t));
    *request = (virtio_block_request_t) {
        .type = rw == VIRTIO_BLOCK_OPERATION_WRITE ? VIRTIO_BLOCK_REQUEST_TYPE_OUT : VIRTIO_BLOCK_REQUEST_TYPE_IN,
        .sector = sector
    };
    virtio_block_device_t* device = &block_devices[block_id];

    // Set descriptors
    unsigned short d1, d2, d3;
    *virtqueue_push_descriptor(device->queue, &d3) = (virtio_descriptor_t) {
        .addr = status,
        .flags = VIRTIO_DESCRIPTOR_FLAG_WRITE_ONLY,
        .length = 1,
        .next = 0
    };

    *virtqueue_push_descriptor(device->queue, &d2) = (virtio_descriptor_t) {
        .addr = data,
        .flags = VIRTIO_DESCRIPTOR_FLAG_NEXT | (rw == VIRTIO_BLOCK_OPERATION_READ ? VIRTIO_DESCRIPTOR_FLAG_WRITE_ONLY : 0),
        .length = size,
        .next = d3
    };

    *virtqueue_push_descriptor(device->queue, &d1) = (virtio_descriptor_t) {
        .addr = request,
        .flags = VIRTIO_DESCRIPTOR_FLAG_NEXT,
        .length = sizeof(virtio_block_request_t),
        .next = d2
    };

    // Set available
    virtqueue_push_available(device->queue, d1);

    // Notify the device of new entries on the queue
    device->mmio->queue_notify = 0;

    return VIRTIO_BLOCK_ERROR_CODE_SUCCESS;
}

// virtio_block_read(unsigned char, unsigned long long, void*, unsigned long long, volatile unsigned char*) -> virtio_block_error_code_t
// Reads sectors from a block device and dumps them into the provided pointer. Status is set to 0xff and remains 0xff until the read is finished.
virtio_block_error_code_t virtio_block_read(unsigned char block_id, unsigned long long sector, void* data, unsigned long long sector_count, volatile unsigned char* status) {
    return virtio_block_operation(VIRTIO_BLOCK_OPERATION_READ, block_id, sector, data, sector_count * 512, status);
}

// virtio_block_write(unsigned char, unsigned long long, void*, unsigned long long, volatile unsigned char*) -> virtio_block_error_code_t
// Reads sectors from a block device and dumps them into the provided pointer. Status is set to 0xff and remains 0xff until the read is finished.
virtio_block_error_code_t virtio_block_write(unsigned char block_id, unsigned long long sector, void* data, unsigned long long sector_count, volatile unsigned char* status) {
    return virtio_block_operation(VIRTIO_BLOCK_OPERATION_WRITE, block_id, sector, data, sector_count * 512, status);
}

char virtio_block_unpack_read(void* buffer, unsigned long long sector, unsigned long long sector_count, unsigned char* metadata) {
    volatile unsigned char status = 0xff;
    if (!virtio_block_read(*metadata, sector, buffer, sector_count, &status)) {
        while (status == 0xff);
        if (status)
            return -1;
        else
            return 0;
    }
    return -1;
}

char virtio_block_unpack_write(void* buffer, unsigned long long sector, unsigned long long sector_count, unsigned char* metadata) {
    volatile unsigned char status = 0xff;
    if (!virtio_block_write(*metadata, sector, buffer, sector_count, &status)) {
        while (status == 0xff);
        if (status)
            return -1;
        else
            return 0;
    }
    return -1;
}

void virtio_block_make_generic(unsigned char block_id, generic_dir_t* dev) {
    generic_block_t* device = malloc(sizeof(generic_block_t));
    *device = (generic_block_t) {
        .unpack_read = virtio_block_unpack_read,
        .unpack_write = virtio_block_unpack_write,
        .used = 1,
        .metadata = { block_id, 0 }
    };

    char name[] = { 'v', 'i', 'r', 't', '-', 'b', 'l', 'k', '0' + block_id, 0 };
    generic_dir_append_entry(dev, (struct s_dir_entry) {
        .name = strdup(name),
        .tag = DIR_ENTRY_TYPE_BLOCK,
        .value = {
            .block = device
        }
    });
}

// clean_virtio_block_devices() -> void
// Cleans up virtio block devices.
void clean_virtio_block_devices() {
    for (unsigned int i = 0; i < VIRTIO_DEVICE_COUNT; i++) {
        virtio_block_device_t device = block_devices[i];
        if (device.in_use) {
            free((void*) device.queue);
        }
    }
}
