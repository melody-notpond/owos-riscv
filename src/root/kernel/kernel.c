#include "uart.h"
#include "virtio.h"

void kmain() {
    while (1);
}

void kinit() {
    uart_puts("Booted into kernel\n");

    virtio_probe();
    uart_puts("Finished initialisation.\n");

    kmain();
}

