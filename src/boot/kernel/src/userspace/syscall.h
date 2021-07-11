#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

// user_syscall(unsigned long long, unsigned long long, unsigned long long, unsigned long long, unsigned long long, unsigned long long, unsigned long long) -> void
// Does a syscall for a user mode process.
void user_syscall(
    unsigned long long syscall,
    unsigned long long a0,
    unsigned long long a1,
    unsigned long long a2,
    unsigned long long a3,
    unsigned long long a4,
    unsigned long long a5
);

#endif /* KERNEL_SYSCALL_H */
