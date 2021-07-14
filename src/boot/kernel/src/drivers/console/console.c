#include <stdarg.h>

#include "console.h"
#include "../../lib/memory.h"
#include "../../lib/printf.h"
#include "../../opensbi.h"

extern generic_filesystem_t console_fs;

// console_generic_file_write(generic_file_t*) -> int
// Reads a character from the console.
int console_generic_file_read(generic_file_t* _) {
    return console_getc();
}

// console_generic_file_write(generic_file_t*, int) -> int
// Writes a character to the console.
int console_generic_file_write(generic_file_t* _, int c) {
    sbi_console_putchar(c);
    return 0;
}

// console_getc_noecho() -> char
// Gets a character from the console without echoing it back.
char console_getc_noecho() {
    int c;
    while ((c = sbi_console_getchar()) == -1);
    return c;
}

// console_getc(void) -> char
// Gets a character from the console and echos it back.
char console_getc() {
    int c;
    while ((c = sbi_console_getchar()) == -1);
    sbi_console_putchar(c);
    return c;
}

// console_puts(char*) -> void
// Puts a string onto the console.
void console_puts(char* s) {
    while (*s) {
        sbi_console_putchar(*s);
        s++;
    }
}

// console_printf(char*, ...) -> void
// Writes its arguments to the UART port according to the format string and write function provided.
__attribute__((format(printf, 1, 2))) void console_printf(char* format, ...) {
    va_list va;
    va_start(va, format);
    func_vprintf(sbi_console_putchar, format, va);
    va_end(va);
}

// console_log_16(unsigned long long) -> unsigned int
// Gets the base 16 log of a number as an int.
unsigned int console_log_16(unsigned long long n) {
    unsigned int i = 0;
    while (n) {
        n >>= 4;
        i++;
    }
    return i;
}

// console_put_hexdump(void*, unsigned long long)
// Dumps a hexdump onto the UART port.
void console_put_hexdump(void* data, unsigned long long size) {
    unsigned int num_zeros = console_log_16(size);
    unsigned char* data_char = (unsigned char*) data;

    for (unsigned long long i = 0; i < (size + 15) / 16; i++) {
        // Print out buffer zeroes
        unsigned int num_zeros_two = num_zeros - console_log_16(i) - 1;
        for (unsigned int j = 0; j < num_zeros_two; j++) {
            console_printf("%x", 0);
        }

        // Print out label
        console_printf("%llx    ", i * 16);

        // Print out values
        for (int j = 0; j < 16; j++) {
            unsigned long long index = i * 16 + j;

            // Skip values if the index is greater than the number of values to dump
            if (index >= size)
                console_puts("   ");
            else {
                // Print out the value
                if (data_char[index] < 16)
                    console_printf("%x", 0);
                console_printf("%x ", data_char[index]);
            }
        }

        console_puts("    |");

        // Print out characters
        for (int j = 0; j < 16; j++) {
            unsigned long long index = i * 16 + j;

            // Skip characters if the index is greater than the number of characters to dump
            if (index >= size)
                sbi_console_putchar('.');

            // Print out printable characters
            else if (32 <= data_char[index] && data_char[index] < 127)
                sbi_console_putchar(data_char[index]);

            // Nonprintable characters are represented by a period (.)
            else
                sbi_console_putchar('.');
        }

        console_puts("|\n");
    }
}

// console_readline(char*) -> char*
// Reads a line from console input.
char* console_readline(char* prompt) {
    console_puts(prompt);
    unsigned long long buffer_size = 16;
    unsigned long long buffer_len = 0;
    char* buffer = malloc(buffer_size);
    buffer[0] = 0;

    while (1) {
        char c = console_getc_noecho();

        if (c == 0x0d)
            break;
        else if (c == 0x7f) {
            if (buffer_len > 0) {
                buffer[--buffer_len] = 0;
                console_puts("\x1b[D \x1b[D");
            }

        // TODO: arrow controls (or maybe not idk)
        } else if (c != 0x1b) {
            sbi_console_putchar(c);
            buffer[buffer_len++] = c;
            if (buffer_len >= buffer_size)
                buffer = realloc(buffer, (buffer_size <<= 1));
            buffer[buffer_len] = 0;
        }
    }

    sbi_console_putchar('\n');
    return buffer;
}

