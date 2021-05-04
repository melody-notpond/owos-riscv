#include "uart.h"

unsigned int uart_log_16(unsigned long long n) {
    unsigned int i = 0;
    while (n) {
        n >>= 4;
        i++;
    }
    return i;
}

void uart_put_hexdump(void* data, unsigned long long size) {
    unsigned int num_zeros = uart_log_16(size);
    unsigned char* data_char = (unsigned char*) data;

    for (unsigned long long i = 0; i < (size + 15) / 16; i++) {
        unsigned int num_zeros_two = num_zeros - uart_log_16(i) - 1;
        for (unsigned int j = 0; j < num_zeros_two; j++) {
            uart_put_hex(0);
        }

        uart_put_hex(i * 16);
        uart_puts("    ");

        for (int j = 0; j < 16; j++) {
            unsigned long long index = i * 16 + j;
            if (index >= size)
                uart_puts("   ");
            else {
                if (data_char[index] < 16)
                    uart_put_hex(0);
                uart_put_hex(data_char[index]);
                uart_putc(' ');
            }
        }

        uart_puts("    |");

        for (int j = 0; j < 16; j++) {
            unsigned long long index = i * 16 + j;
            if (index >= size)
                uart_putc('.');
            else if (32 <= data_char[index] && data_char[index] < 127)
                uart_putc(data_char[index]);
            else
                uart_putc('.');
        }

        uart_puts("|\n");
    }
}
