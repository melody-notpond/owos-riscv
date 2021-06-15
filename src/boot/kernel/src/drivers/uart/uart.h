#ifndef KERNEL_UART_H
#define KERNEL_UART_H

// uart_puts(char*) -> void
// Puts a string onto the UART port.
void uart_puts(char*);

// uart_putc(char) -> void
// Puts a character onto the UART port.
void uart_putc(char);

// uart_getc(void) -> char
// Gets a character from the UART port and echos it back.
void uart_getc();

// uart_getc_noecho(void) -> char
// Gets a character from the UART port without echoing it back.
char uart_getc_noecho();

// uart_printf(char*, ...) -> void
// Writes its arguments to the UART port according to the format string and write function provided.
__attribute__((format(printf, 1, 2))) void uart_printf(char* format, ...);

// uart_put_hexdump(void*, unsigned long long)
// Dumps a hexdump onto the UART port.
void uart_put_hexdump(void* data, unsigned long long size);

// uart_readline(char*) -> char*
// Reads a line from uart input.
char* uart_readline(char* prompt);

#endif /* KERNEL_UART_H */

