#ifndef KERNEL_UART_H
#define KERNEL_UART_H

void uart_puts(char*);
void uart_putc(char);
void uart_put_hex(long long);
void uart_getc();
void uart_put_hexdump(void* data, unsigned long long size);

#endif /* KERNEL_UART_H */

