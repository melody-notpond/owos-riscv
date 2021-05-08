#ifndef KERNEL_PRINTF_H
#define KERNEL_PRINTF_H

#include <stdarg.h>

// func_printf(void (*)(char), char*, ...) -> void
// Writes its arguments according to the format string and write function provided.
__attribute__((format(printf, 2, 3))) void func_printf(void (*write)(char), char* format, ...);

// func_vprintf(void (*)(char), char*, ...) -> void
// Writes its arguments according to the format string and write function provided. Takes in a va_list.
void func_vprintf(void (*write)(char), char* format, va_list va);

#endif /* KERNEL_PRINTF_H */


