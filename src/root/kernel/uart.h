#ifndef KERNEL_UART_H
#define KERNEL_UART_H

void uart_puts(char*);
void uart_putc(char);
void uart_put_hex(long long);
void uart_getc();

#endif /* KERNEL_UART_H */

