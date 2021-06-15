#include "drivers/uart/uart.h"
#include "kshell.h"
#include "lib/memory.h"

// kshell_main() -> void
// Does the shell.
void kshell_main() {
    while (1) {
        char* s = uart_readline("> ");
        uart_printf("%s\n", s);
        free(s);
    }
}

