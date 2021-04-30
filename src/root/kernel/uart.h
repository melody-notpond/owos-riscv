#ifndef KERNEL_UART_H
#define KERNEL_UART_H

void uart_puts(char*);
void uart_putc(char _, char);
void uart_put_hex(long long);

#endif /* KERNEL_UART_H */

