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

    char buffer[100];
    int fd = syscall_wrapper(2, (unsigned long long) "/etc/fstab", 0, 0, 0, 0, 0);
    syscall_wrapper(0, fd, (unsigned long long) buffer, 100, 0, 0, 0);
    syscall_wrapper(3, fd, 0, 0, 0, 0, 0);
    syscall_wrapper(1, 1, (unsigned long long) buffer, 100, 0, 0, 0);

    syscall_wrapper(60, 0, 0, 0, 0, 0, 0);
}
