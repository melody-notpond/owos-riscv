#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

// user_syscall(unsigned long long) -> void
// Does a syscall for a user mode process.
void user_syscall(unsigned long long syscall);

#endif /* KERNEL_SYSCALL_H */
