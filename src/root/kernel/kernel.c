void uart_puts(char*);
void uart_putc(char _, char);
void uart_put_hex(long long);

void kinit() {
    uart_puts("Booted into kernel\n");
    while (1);
}
