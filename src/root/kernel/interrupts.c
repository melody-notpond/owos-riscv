#include "uart.h"

#define INTERRUPT_DEBUG
#define PLIC_BASE  0x0c000000
#define PLIC_CLAIM 0x0c200004

// Interrupt handlers
void (*mei_interrupt_handlers[53])(unsigned int) = { 0 };

// register_mei_handler(unsigned int, unsigned char, void (*)(unsigned int)) -> char
// Registers a machine external interrupt with a given mei id, priority, and handler. If the priority is 0, then the interrupt is disabled. Returns 0 on successful registration, 1 on failure.
char register_mei_handler(unsigned int mei_id, unsigned char priority, void (*mei_handler)(unsigned int)) {
    // Register
    if (0 < mei_id && mei_id <= 53 && mei_interrupt_handlers[mei_id - 1] == 0) {
        mei_interrupt_handlers[mei_id - 1] = mei_handler;
        *(((unsigned int*) PLIC_BASE) + mei_id) = priority;
        return 0;
    }
    return 1;
}

// handle_mei(void) -> void
// Handles a machine external interrupt.
void handle_mei() {
    // Claim the interrupt
    volatile unsigned int* claim_reg = (volatile unsigned int*) PLIC_CLAIM;
    unsigned int mei_id = *claim_reg;
    *claim_reg = mei_id;

    // Debug stuff
#ifdef INTERRUPT_DEBUG
    uart_puts("MEI id is 0x");
    uart_put_hex(mei_id);
    uart_putc('\n');
#endif

    // Call handler if available
    void (*mei_handler)(unsigned int) = mei_interrupt_handlers[mei_id - 1];
    if (mei_handler)
        mei_handler(mei_id);

}

// handle_interrupt(unsigned long long) -> void
// Called by the interrupt handler to dispatch the interrupt.
void handle_interrupt(unsigned long long mcause) {
    // Debug stuff
#ifdef INTERRUPT_DEBUG
    uart_puts("Interrupt received: 0x");
    uart_put_hex(mcause);
    uart_putc('\n');
#endif

    // Asynchronous interrupts
    if (mcause &  0x8000000000000000) {
        mcause &= 0x7fffffffffffffff;
        switch (mcause) {
            case 0x0b:
                handle_mei();
                break;
            default:
                break;
        }

    // Synchronous interrupts
    } else {
        uart_puts("synchronous interrupt\n");
        switch (mcause) {
            default:
                break;
        }
    }
}

