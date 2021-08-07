#include <stdarg.h>
#include "printf.h"

typedef enum {
    SIZE_INT,
    SIZE_LONG,
    SIZE_LONG_LONG
} printf_size_t;

// func_printf(void (*)(char), char*, ...) -> void
// Writes its arguments according to the format string and write function provided.
__attribute__((format(printf, 2, 3))) void func_printf(void (*write)(char), char* format, ...) {
    va_list va;
    va_start(va, format);
    func_vprintf(write, format, va);
    va_end(va);
}

// func_vprintf(void (*)(char), char*, ...) -> void
// Writes its arguments according to the format string and write function provided. Takes in a va_list.
void func_vprintf(void (*write)(char), char* format, va_list va) {
    for (; *format; format++) {
        // Formatters
        if (*format == '%') {
            format++;

            // Determine the type of the formatter
            printf_size_t size = SIZE_INT;
            switch (*format) {
                case 'c':
                    write(va_arg(va, int));
                    break;

                case 'p': {
                    unsigned long long p = (unsigned long long) va_arg(va, void*);
                    write('0');
                    write('x');
                    for (unsigned long long i = 15 * 4; i; i -= 4) {
                        write("0123456789abcdef"[(p & (0xf << i)) >> i]);
                    }
                    write("0123456789abcdef"[p & 0xf]);
                    break;
                }

                case 's': {
                    char* s = va_arg(va, char*);
                    for (; *s; s++) {
                        write(*s);
                    }
                    break;
                }

                case 'l':
                    size = SIZE_LONG;
                    format++;
                    if (*format == 'l') {
                        format++;
                        size = SIZE_LONG_LONG;
                    }

                    if (*format != 'x') {
                        switch (size) {
                            case SIZE_INT:
                                va_arg(va, unsigned int);
                                break;
                            case SIZE_LONG:
                                va_arg(va, unsigned long);
                                break;
                            case SIZE_LONG_LONG:
                                va_arg(va, unsigned long long);
                                break;
                        }
                        format--;
                        break;
                    }

                case 'x': {
                    unsigned long long x;
                    switch (size) {
                        case SIZE_INT:
                            x = va_arg(va, unsigned int);
                            break;
                        case SIZE_LONG:
                            x = va_arg(va, unsigned long);
                            break;
                        case SIZE_LONG_LONG:
                            x = va_arg(va, unsigned long long);
                            break;
                    }

                    char writing = 0;
                    for (unsigned long long i = 15 * 4; i; i -= 4) {
                        unsigned long long c = ((x & (0xf << i)) >> i) & 0xf;
                        writing = writing || c != 0;
                        if (writing)
                            write("0123456789abcdef"[c]);
                    }
                    write("0123456789abcdef"[x & 0xf]);
                    break;
                }

                case '%':
                    write('%');
                    break;

                default:
                    break;
            }

        // Regular characters
        } else {
            write(*format);
        }
    }
}
