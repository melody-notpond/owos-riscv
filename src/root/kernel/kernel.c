#include "uart.h"
#include "virtio/virtio.h"

void kmain() {
    while (1);
}

void kinit() {
    uart_puts("Booted into kernel\n");

    init_heap_metadata();

    virtio_probe();
    uart_puts("Finished initialisation.\n");

    kmain();
}

