#ifndef KERNEL_VIRTIO_BLOCK_H
#define KERNEL_VIRTIO_BLOCK_H

#include "virtio.h"

typedef struct __attribute__((__packed__, aligned(4))) { 
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
    unsigned short idx;
    unsigned short ack_used_idx;
    char in_use;
    char ro;
} virtio_block_device_t;

extern virtio_block_device_t block_devices[VIRTIO_DEVICE_COUNT];

char virtio_init_block_device(volatile virtio_mmio_t* mmio);

#endif /* KERNEL_VIRTIO_BLOCK_H */

