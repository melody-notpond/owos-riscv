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
    syscall_wrapper(69, 0, 1, 2, 3, 4, 5);
    while (1);
}

