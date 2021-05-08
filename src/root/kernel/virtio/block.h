#ifndef KERNEL_VIRTIO_BLOCK_H
#define KERNEL_VIRTIO_BLOCK_H

#include "../generic_block.h"
#include "virtio.h"
#include "virtqueue.h"

char virtio_init_block_device(volatile virtio_mmio_t* mmio);

typedef enum {
    VIRTIO_BLOCK_ERROR_CODE_SUCCESS = 0,
    VIRTIO_BLOCK_ERROR_CODE_INVALID_DEVICE,
    VIRTIO_BLOCK_ERROR_CODE_NOT_BLOCK_DEVICE,
    VIRTIO_BLOCK_ERROR_CODE_READ_ONLY,
    VIRTIO_BLOCK_ERROR_CODE_OPERATION_BEYOND_CAPACITY
} virtio_block_error_code_t;

// virtio_block_read(unsigned char, unsigned long long, void*, unsigned long long, volatile unsigned char*) -> virtio_block_error_code_t
// Reads sectors from a block device and dumps them into the provided pointer. Status is set to 0xff and remains 0xff until the read is finished.
virtio_block_error_code_t virtio_block_read(unsigned char block_id, unsigned long long sector, void* data, unsigned long long sector_count, volatile unsigned char* status);

// virtio_block_write(unsigned char, unsigned long long, void*, unsigned long long, volatile unsigned char*) -> virtio_block_error_code_t
// Reads sectors from a block device and dumps them into the provided pointer. Status is set to 0xff and remains 0xff until the read is finished.
virtio_block_error_code_t virtio_block_write(unsigned char block_id, unsigned long long sector, void* data, unsigned long long sector_count, volatile unsigned char* status);

void virtio_block_make_generic(unsigned char block_id, generic_dir_t* dev);

#endif /* KERNEL_VIRTIO_BLOCK_H */

