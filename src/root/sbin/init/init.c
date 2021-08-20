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
    syscall_wrapper(1, 1, (unsigned long long) "Init process started\n", 21, 0, 0, 0);
    syscall_wrapper(314, (unsigned long long) "/bin/lil", 0, 0, 0, 1, 2);
    syscall_wrapper(1, 1, (unsigned long long) "Spawned lil\n", 12, 0, 0, 0);
    while (1);
}

