#ifndef KERNEL_INTERRUPTS_H
#define KERNEL_INTERRUPTS_H

// register_mei_handler(unsigned int, unsigned char, void (*)(unsigned int)) -> char
// Registers a machine external interrupt with a given mei id, priority, and handler. If the priority is 0, then the interrupt is disabled. Returns 0 on successful registration, 1 on failure.
char register_mei_handler(unsigned int mei_id, unsigned char priority, void (*mei_handler)(unsigned int));

#endif /* KERNEL_INTERRUPTS_H */

