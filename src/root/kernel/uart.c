#include "uart.h"

// uart_log_16(unsigned long long) -> unsigned int
// Gets the base 16 log of a number as an int.
unsigned int uart_log_16(unsigned long long n) {
    unsigned int i = 0;
    while (n) {
        n >>= 4;
        i++;
    }
    return i;
}

// uart_put_hexdump(void*, unsigned long long)
// Dumps a hexdump onto the UART port.
void uart_put_hexdump(void* data, unsigned long long size) {
    unsigned int num_zeros = uart_log_16(size);
    unsigned char* data_char = (unsigned char*) data;

    for (unsigned long long i = 0; i < (size + 15) / 16; i++) {
        // Print out buffer zeroes
        unsigned int num_zeros_two = num_zeros - uart_log_16(i) - 1;
        for (unsigned int j = 0; j < num_zeros_two; j++) {
            uart_put_hex(0);
        }

        // Print out label
        uart_put_hex(i * 16);
        uart_puts("    ");

        // Print out values
        for (int j = 0; j < 16; j++) {
            unsigned long long index = i * 16 + j;

            // Skip values if the index is greater than the number of values to dump
            if (index >= size)
                uart_puts("   ");
            else {
                // Print out the value
                if (data_char[index] < 16)
                    uart_put_hex(0);
                uart_put_hex(data_char[index]);
                uart_putc(' ');
            }
        }

        uart_puts("    |");

        // Print out characters
        for (int j = 0; j < 16; j++) {
            unsigned long long index = i * 16 + j;

            // Skip characters if the index is greater than the number of characters to dump
            if (index >= size)
                uart_putc('.');

            // Print out printable characters
            else if (32 <= data_char[index] && data_char[index] < 127)
                uart_putc(data_char[index]);

            // Nonprintable characters are represented by a period (.)
            else
                uart_putc('.');
        }

        uart_puts("|\n");
    }
}
