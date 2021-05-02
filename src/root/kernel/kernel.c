#include "memory.h"
#include "uart.h"
#include "virtio/block.h"
#include "virtio/virtio.h"

void kmain() {
    uart_puts("Finished initialisation.\n");

    // Temporary block writing tests
    volatile unsigned char status;
    uart_puts("Writing test data to block.\n");
    virtio_block_write(7, 0, "hewwo uwu aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 1, &status);
    while (status == 0xff);
    uart_puts("Wrote to block device.\n");
    uart_puts("Reading test data from block\n");
    char data[513];
    data[512] = 0;
    virtio_block_read(7, 0, data, 1, &status);
    while (status == 0xff);
    uart_puts("Read data to memory: ");
    uart_puts(data);
    uart_putc('\n');

    // Hang
    while (1) {
        uart_getc();
    }
}

void kinit() {
    uart_puts("Initialising kernel\n");

    // Initialise heap
    init_heap_metadata();

    // Probe for available virtio devices
    virtio_probe();

    // Jump to interrupt init code
    asm("j interrupt_init");
}

