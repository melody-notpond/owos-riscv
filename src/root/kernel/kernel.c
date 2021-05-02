#include "uart.h"
#include "virtio/block.h"
#include "virtio/virtio.h"

void kmain() {
    uart_puts("Writing test data to block.\n");
    virtio_block_write(7, 0, "hewwo uwu aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 512);
    uart_puts("Wrote to block device.\n");
    uart_puts("Reading test data to block\n");
    char data[513];
    data[512] = 0;
    virtio_block_read(7, 0, data, 512);
    uart_puts("Read data to memory: ");
    uart_puts(data);
    uart_putc('\n');

    while (1) {
        uart_getc();
    }
}

void kinit() {
    uart_puts("Booted into kernel\n");

    init_heap_metadata();

    virtio_probe();
    uart_puts("Finished initialisation.\n");

    kmain();
}

