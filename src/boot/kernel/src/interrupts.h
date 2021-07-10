#ifndef KERNEL_INTERRUPTS_H
#define KERNEL_INTERRUPTS_H

#define PLIC_BASE  0x0c000000
#define PLIC_CLAIM 0x0c200004
#define PLIC_COUNT 53

// register_mei_handler(unsigned int, unsigned char, void (*)(unsigned int)) -> char
// Registers a machine external interrupt with a given mei id, priority, and handler. If the priority is 0, then the interrupt is disabled. Returns 0 on successful registration, 1 on failure.
char register_mei_handler(unsigned int mei_id, unsigned char priority, void (*mei_handler)(unsigned int));

#endif /* KERNEL_INTERRUPTS_H */

