unsigned long long syscall_wrapper(
    unsigned long long syscall,
    unsigned long long a0,
    unsigned long long a1,
    unsigned long long a2,
    unsigned long long a3,
    unsigned long long a4,
    unsigned long long a5
);

int main(int argc, char** argv) {
    syscall_wrapper(1, 1, (unsigned long long) "Simple process started\n", 23, 0, 0, 0);
    syscall_wrapper(0x69, 0, 1, 2, 3, 4, 5);
    syscall_wrapper(60, 0, 0, 0, 0, 0, 0);
    syscall_wrapper(1, 1, (unsigned long long) "Welp I messed up :(\n", 20, 0, 0, 0);
}
