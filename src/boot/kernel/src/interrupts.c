#include "interrupts.h"
#include "opensbi.h"
#include "userspace/syscall.h"
#include "drivers/console/console.h"

//#define INTERRUPT_DEBUG

// Interrupt handlers
void (*mei_interrupt_handlers[PLIC_COUNT])(unsigned int) = { 0 };

// get_context_enable_bits(unsigned long long) -> volatile unsigned int*
// Gets a volatile pointer to the interrupt enable bits for a given context.
volatile unsigned int* get_context_enable_bits(unsigned long long context) {
    return (volatile void*) (PLIC_BASE + PLIC_ENABLES_OFFSET + context * 0x80);
}

// get_context_priority_threshold(unsigned long long) -> volatile unsigned int*
// Gets a volatile pointer to the priority threshold for a given context.
volatile unsigned int* get_context_priority_threshold(unsigned long long context) {
    return (volatile void*) (PLIC_BASE + PLIC_THRESHOLD_OFFSET + context * 0x1000);
}

// get_context_claim_pointer(unsigned long long) -> volatile unsigned int*
// Gets a volatile pointer to the interupt claim register for a given context.
volatile unsigned int* get_context_claim_pointer(unsigned long long context) {
    return (volatile void*) (PLIC_BASE + PLIC_CLAIM_OFFSET + context * 0x1000);
}

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
    volatile unsigned int* claim_reg = get_context_claim_pointer(PLIC_CONTEXT(0, 1)); // TODO: don't hardcode the context
    unsigned int mei_id = *claim_reg;
    *claim_reg = mei_id;

    // Debug stuff
#ifdef INTERRUPT_DEBUG
    console_printf("MEI id is 0x%x\n", mei_id);
#endif

    // Call handler if available
    void (*mei_handler)(unsigned int) = mei_interrupt_handlers[mei_id - 1];
    if (mei_handler)
        mei_handler(mei_id);
}

// swap_process(trap_t*) -> void
// Swaps the current process with the next process in the queue.
void swap_process(trap_t* trap) {
    // Save current state
    process_t* process = fetch_process(trap->pid);
    process->pc = trap->pc;
    memcpy(process->xs, trap->xs, sizeof(unsigned long long) * 32);
    memcpy(process->fs, trap->fs, sizeof(double) * 32);

    pid_t pid = next_process_in_queue();
    if (pid) {
        // Get process
        process_t* new = fetch_process(pid);
        trap->pid = pid;

        // Set mmu
        unsigned long long mmu = (((unsigned long long) new->mmu_data) >> 12) | 0x8000000000000000;
        asm volatile("csrw satp, %0" : "=r" (mmu));
        mmu = 0;
        asm volatile("sfence.vma zero, %0" : "=r" (mmu));

        // Set trap registers
        trap->pc = new->pc;
        memcpy(trap->xs, new->xs, sizeof(unsigned long long) * 32);
        memcpy(trap->fs, new->fs, sizeof(double) * 32);

        // Set ring to user ring
        mmu = 0x100;
        asm volatile("csrc sstatus, %0" : "=r" (mmu));
    }
}

// handle_interrupt(unsigned long long, unsigned long long, struct s_trap, pid_t) -> trap_t*
// Called by the interrupt handler to dispatch the interrupt. Returns the trap structure to jump back to.
trap_t* handle_interrupt(unsigned long long scause, trap_t* trap) {
    // Debug stuff
#ifdef INTERRUPT_DEBUG
    console_printf("Interrupt received: 0x%llx\n", scause);
#endif

    // Asynchronous interrupts
    if (scause &  0x8000000000000000) {
        scause &= 0x7fffffffffffffff;
        switch (scause) {
            // Timer interrupts
            case 0x05: {
                swap_process(trap);

                unsigned long long time = 0;
                asm volatile("csrr %0, time" : "=r" (time));
                sbi_set_timer(time + 10000);
                break;
            }

            // External interrupts
            case 0x09:
                handle_mei();
                break;

            default:
                console_printf("unknown asynchronous interrupt: 0x%llx\n", scause);
                while (1);
        }

    // Synchronous interrupts
    } else {
        switch (scause) {
            // User mode syscall
            case 0x08:
                trap->xs[PROCESS_REGISTER_A0] = user_syscall(
                    trap->pid,
                    trap->xs[PROCESS_REGISTER_A7],
                    trap->xs[PROCESS_REGISTER_A0],
                    trap->xs[PROCESS_REGISTER_A1],
                    trap->xs[PROCESS_REGISTER_A2],
                    trap->xs[PROCESS_REGISTER_A3],
                    trap->xs[PROCESS_REGISTER_A4],
                    trap->xs[PROCESS_REGISTER_A5]
                );
                trap->pc += 4;
                break;

            default:
                console_printf("unknown synchronous interrupt: 0x%llx\n", scause);
                while (1);
        }
    }

    return trap;
}

