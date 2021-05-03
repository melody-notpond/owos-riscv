#include "gpu.h"
#include "../uart.h"

char virtio_init_graphics_device(volatile virtio_mmio_t* mmio) {
    VIRTIO_GENERIC_INIT(mmio, );
    VIRTIO_GENERIC_INIT_FINISH(mmio);
    return 0;
}
