#include "syscall.h"
#include "../drivers/console/console.h"
#include "../opensbi.h"
#include "../drivers/filesystems/generic_file.h"

// user_syscall(pid_t, unsigned long long, unsigned long long, unsigned long long, unsigned long long, unsigned long long, unsigned long long, unsigned long long) -> unsigned long long
// Does a syscall for a user mode process.
unsigned long long user_syscall(
    pid_t pid,
    unsigned long long syscall,
    unsigned long long a0,
    unsigned long long a1,
    unsigned long long a2,
    unsigned long long a3,
    unsigned long long a4,
    unsigned long long a5
) {
    switch (syscall) {
        // unsigned long long write(unsigned int fd, char* buffer, unsigned long long count);
        case 1: {
            unsigned int fd = a0;
            char* buffer = (void*) a1;
            unsigned long long count = a2;
            process_t* process = fetch_process(pid);
            generic_file_t** file_descriptors = process->file_descriptors;

            // TODO: make this not hardcoded
            if (fd == 1) {
                for (unsigned long long i = 0; i < count; i++) {
                    sbi_console_putchar(buffer[i]);
                }
                return count;
            }

            if (fd >= 1024)
                return -1;
            generic_file_t* file = file_descriptors[fd];
            if (file == (void*) 0)
                return -1;

            // TODO
            return -1;

            // return count;
        }

        // pid_t getpid(void);
        case 39: {
            return pid;
        }

        // pid_t getppid(void);
        case 110: {
            return fetch_process(pid)->parent_pid;
        }

        // pid_t spawn(char* path, char* argv[], char* envp[]);
        case 314: {
            char* path = (void*) a0;

            // TODO: use these
            char** argv = (void*) a1;
            char** envp = (void*) a2;

            elf_t elf = load_executable_elf_from_file(root, path);
            pid_t p = load_elf_as_process(pid, &elf, 1);
            free_elf(&elf);
            process_init_kernel_mmu(p);
            add_process_to_queue(p);
            return 0;
        }

        // Unknown syscall
        default:
            console_printf("Called unknown syscall 0x%llx with arguments:\n", syscall);
            console_printf("    a0: 0x%llx\n", a0);
            console_printf("    a1: 0x%llx\n", a1);
            console_printf("    a2: 0x%llx\n", a2);
            console_printf("    a3: 0x%llx\n", a3);
            console_printf("    a4: 0x%llx\n", a4);
            console_printf("    a5: 0x%llx\n", a5);
            return -1;
    }
}

