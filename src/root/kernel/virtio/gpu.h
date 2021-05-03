#ifndef KERNEL_VIRTIO_GPU_H
#define KERNEL_VIRTIO_GPU_H

#include "virtio.h"

char virtio_init_graphics_device(volatile virtio_mmio_t* mmio);

#endif /* KERNEL_VIRTIO_GPU_H */

