static inline void syscall_wrapper(
    unsigned long long syscall,
    unsigned long long a0,
    unsigned long long a1,
    unsigned long long a2,
    unsigned long long a3,
    unsigned long long a4,
    unsigned long long a5
) {
    asm("mv a7, %0" : "=r" (syscall));
    asm("mv a0, %0" : "=r" (a0));
    asm("mv a1, %0" : "=r" (a1));
    asm("mv a2, %0" : "=r" (a2));
    asm("mv a3, %0" : "=r" (a3));
    asm("mv a4, %0" : "=r" (a4));
    asm("mv a5, %0" : "=r" (a5));
    asm("ecall");
}

int main(int argc, char** argv) {
    syscall_wrapper(69, 0, 1, 2, 3, 4, 5);
    while (1);
}
