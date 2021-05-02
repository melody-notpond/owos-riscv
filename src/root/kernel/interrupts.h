#ifndef KERNEL_INTERRUPTS_H
#define KERNEL_INTERRUPTS_H

char register_mei_handler(unsigned int mei_id, unsigned char priority, void (*mei_handler)(unsigned int));

#endif /* KERNEL_INTERRUPTS_H */

