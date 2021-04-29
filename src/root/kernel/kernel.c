#define UART_BASE 0x10010000

void uart_puts(char* string) {
    while (*string != 0) {
        while (*((char*) UART_BASE) != 0);

        *((char*) UART_BASE) = *string;
        string++;
    }
}

void kmain() {
    uart_puts("Booted into kernel");
    while (1);
}
