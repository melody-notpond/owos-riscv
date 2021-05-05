#ifndef KERNEL_UART_H
#define KERNEL_UART_H

// uart_puts(char*) -> void
// Puts a string onto the UART port.
void uart_puts(char*);

// uart_putc(char) -> void
// Puts a character onto the UART port.
void uart_putc(char);

// uart_put_hex(long long) -> void
// Puts a long long as a hex number into the UART port.
void uart_put_hex(long long);

// uart_getc(void) -> char
// Gets a character from the UART port and echos it back.
void uart_getc();

// uart_put_hexdump(void*, unsigned long long)
// Dumps a hexdump onto the UART port.
void uart_put_hexdump(void* data, unsigned long long size);

#endif /* KERNEL_UART_H */

