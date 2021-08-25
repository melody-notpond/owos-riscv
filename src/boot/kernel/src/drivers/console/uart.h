#ifndef UART_H
#define UART_H

#include "../devicetree/tree.h"

typedef struct s_uart_mmio uart_mmio_t;

// init_uart(fdt_t*, char*) -> uart_mmio_t*
// Initialises a UART port. Returns 0 on success.
uart_mmio_t* init_uart(fdt_t* fdt, char* path);

// uart_write_str(uart_mmio_t*, char*, unsigned long long, void (*)(void*), void*) -> int
// Writes a string to the uart. Returns 0 on success.
int uart_write_str(uart_mmio_t* mmio, char* data, unsigned long long length, void (*callback_fn)(void*), void* callback_data);

#endif /* UART_H */

