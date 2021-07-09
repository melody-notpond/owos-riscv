#include "gpu.h"
#include "../../lib/memory.h"
#include "../console/console.h"
#include "virtqueue.h"

// virtio_init_graphics_device(volatile virtio_mmio_t*) -> char
// Initialises a virtio graphics device.
char virtio_init_graphics_device(volatile virtio_mmio_t* mmio) {
    VIRTIO_GENERIC_INIT(mmio, );

    volatile virtio_queue_t* controlq = virtqueue_add_to_device(mmio, 0);
    if (controlq == 0)
        return -1;

    volatile virtio_queue_t* cursorq = virtqueue_add_to_device(mmio, 1);
    if (cursorq == 0) {
        free((void*) controlq);
        return -1;
    }

    VIRTIO_GENERIC_INIT_FINISH(mmio);
    return 0;
}
