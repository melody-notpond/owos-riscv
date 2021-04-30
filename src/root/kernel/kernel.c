#include "uart.h"

void kinit() {
    uart_puts("Booted into kernel\n");

    virtio_probe();

    while (1);
}
