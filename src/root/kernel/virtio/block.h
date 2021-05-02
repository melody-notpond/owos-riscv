#ifndef KERNEL_VIRTIO_BLOCK_H
#define KERNEL_VIRTIO_BLOCK_H

#include "virtio.h"

#define SECTOR_SIZE 512

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

extern virtio_block_device_t block_devices[VIRTIO_DEVICE_COUNT];

char virtio_init_block_device(volatile virtio_mmio_t* mmio);

typedef enum {
    VIRTIO_BLOCK_ERROR_CODE_SUCCESS = 0,
    VIRTIO_BLOCK_ERROR_CODE_INVALID_DEVICE,
    VIRTIO_BLOCK_ERROR_CODE_NOT_BLOCK_DEVICE,
    VIRTIO_BLOCK_ERROR_CODE_READ_ONLY,
    VIRTIO_BLOCK_ERROR_CODE_OPERATION_BEYOND_CAPACITY
} virtio_block_error_code_t;

virtio_block_error_code_t virtio_block_read(unsigned char block_id, unsigned long long sector, void* data, unsigned long long size);

virtio_block_error_code_t virtio_block_write(unsigned char block_id, unsigned long long sector, void* data, unsigned long long size);

#endif /* KERNEL_VIRTIO_BLOCK_H */

