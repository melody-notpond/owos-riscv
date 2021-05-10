#include "syscall.h"
#include "uart.h"

// user_syscall(unsigned long long) -> void
// Does a syscall for a user mode process.
void user_syscall(unsigned long long syscall) {
    switch (syscall) {
        case 0x00:
            uart_puts("Called syscall 0! (Not factorial)\n");
            break;
        default:
            uart_printf("Called unknown syscall %llx.\n", syscall);
            break;
    }
}

