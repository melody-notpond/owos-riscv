#include "interrupts.h"
#include "userspace/process.h"
#include "userspace/syscall.h"
#include "drivers/console/console.h"

//#define INTERRUPT_DEBUG

// Interrupt handlers
void (*mei_interrupt_handlers[53])(unsigned int) = { 0 };

// register_mei_handler(unsigned int, unsigned char, void (*)(unsigned int)) -> char
// Registers a machine external interrupt with a given mei id, priority, and handler. If the priority is 0, then the interrupt is disabled. Returns 0 on successful registration, 1 on failure.
char register_mei_handler(unsigned int mei_id, unsigned char priority, void (*mei_handler)(unsigned int)) {
    // Register
    if (0 < mei_id && mei_id <= PLIC_COUNT && mei_interrupt_handlers[mei_id - 1] == 0) {
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
    console_puts("MEI id is 0x");
    console_put_hex(mei_id);
    console_putc('\n');
#endif

    // Call handler if available
    void (*mei_handler)(unsigned int) = mei_interrupt_handlers[mei_id - 1];
    if (mei_handler)
        mei_handler(mei_id);
}

struct s_trap {
    unsigned long long xs[32];
};

// handle_interrupt(unsigned long long, unsigned long long, struct s_trap, pid_t) -> unsigned long long
// Called by the interrupt handler to dispatch the interrupt. Returns the new value of sepc.
unsigned long long handle_interrupt(unsigned long long scause, unsigned long long sepc, struct s_trap* trap_struct, pid_t pid) {
    // Debug stuff
#ifdef INTERRUPT_DEBUG
    console_puts("Interrupt received: 0x");
    console_put_hex(scause);
    console_putc('\n');
#endif

    // Asynchronous interrupts
    if (scause &  0x8000000000000000) {
        scause &= 0x7fffffffffffffff;
        switch (scause) {
            case 0x0b:
                handle_mei();
                break;
            default:
                break;
        }

    // Synchronous interrupts
    } else {
        switch (scause) {
            // User mode syscall
            case 0x08:
                trap_struct->xs[PROCESS_REGISTER_A0] = user_syscall(
                    pid,
                    trap_struct->xs[PROCESS_REGISTER_A7],
                    trap_struct->xs[PROCESS_REGISTER_A0],
                    trap_struct->xs[PROCESS_REGISTER_A1],
                    trap_struct->xs[PROCESS_REGISTER_A2],
                    trap_struct->xs[PROCESS_REGISTER_A3],
                    trap_struct->xs[PROCESS_REGISTER_A4],
                    trap_struct->xs[PROCESS_REGISTER_A5]
                );
                sepc += 4;
                break;

            default:
                console_printf("unknown synchronous interrupt: 0x%llx\n", scause);
                while (1);
        }
    }

    return sepc;
}

