#ifndef KERNEL_INTERRUPTS_H
#define KERNEL_INTERRUPTS_H

#include "userspace/process.h"

// TODO: Detect base address using device trees
#define PLIC_BASE               0x0c000000
#define PLIC_PRIORITY_OFFSET    0x00000004
#define PLIC_PENDING_OFFSET     0x00001000
#define PLIC_ENABLES_OFFSET     0x00002000
#define PLIC_THRESHOLD_OFFSET   0x00200000
#define PLIC_CLAIM_OFFSET       0x00200004
#define PLIC_COUNT                    1023

// Converts a hartid and supervisor/user mode pair into a context for the PLIC.
#define PLIC_CONTEXT(hartid, s) ((hartid) * 2 + (s))

typedef struct {
    unsigned long long hartid;
    pid_t pid;
    unsigned long long pc;
    unsigned long long xs[32];
    double fs[32];
} trap_t;

// get_context_enable_bits(unsigned long long) -> volatile unsigned int*
// Gets a volatile pointer to the interrupt enable bits for a given context.
volatile unsigned int* get_context_enable_bits(unsigned long long context);

// get_context_priority_threshold(unsigned long long) -> volatile unsigned int*
// Gets a volatile pointer to the priority threshold for a given context.
volatile unsigned int* get_context_priority_threshold(unsigned long long context);

// get_context_claim_pointer(unsigned long long) -> volatile unsigned int*
// Gets a volatile pointer to the interupt claim register for a given context.
volatile unsigned int* get_context_claim_pointer(unsigned long long context);

// register_mei_handler(unsigned int, unsigned char, void (*)(unsigned int)) -> char
// Registers a machine external interrupt with a given mei id, priority, and handler. If the priority is 0, then the interrupt is disabled. Returns 0 on successful registration, 1 on failure.
char register_mei_handler(unsigned int mei_id, unsigned char priority, void (*mei_handler)(unsigned int));

// swap_process(trap_t*) -> void
// Swaps the current process with the next process in the queue.
void swap_process(trap_t* trap);

#endif /* KERNEL_INTERRUPTS_H */

