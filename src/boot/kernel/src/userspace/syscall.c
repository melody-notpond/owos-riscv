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
    unsigned long long a5,
    trap_t* trap
) {
    switch (syscall) {
        // unsigned long long read(int fd, void* buffer, unsigned long long count);
        case 0: {
            int fd = (int) a0;
            void* buffer = (void*) a1;
            unsigned long long count = a2;

            if (fd > FILE_DESCRIPTOR_COUNT)
                return -1;

            process_t* process = fetch_process(pid);

            if (process->file_descriptors[fd] == (void*) 0)
                return -1;

            return generic_file_read(process->file_descriptors[fd], buffer, count);
        }

        // unsigned long long write(int fd, char* buffer, unsigned long long count);
        case 1: {
            int fd = (int) a0;
            void* buffer = (void*) a1;
            unsigned long long count = a2;

            if (fd > FILE_DESCRIPTOR_COUNT)
                return -1;

            process_t* process = fetch_process(pid);

            if (process->file_descriptors[fd] == (void*) 0)
                return -1;

            return generic_file_write(process->file_descriptors[fd], buffer, count);
        }

        // int open(char* path, int flags, int mode);
        case 2: {
            char* path = (void*) a0;

            // TODO: use these
            int flags = (int) a1;
            int mode = (int) a2;

            struct s_dir_entry entry = generic_dir_lookup(root, path);
            if (entry.file == (void*) 0)
                return -1;

            process_t* process = fetch_process(pid);
            for (int i = 3; i < FILE_DESCRIPTOR_COUNT; i++) {
                if (process->file_descriptors[i] == (void*) 0) {
                    process->file_descriptors[i] = entry.file;
                    return i;
                }
            }
            return -1;
        }

        // int close(int fd)
        case 3: {
            int fd = (int) a0;

            if (fd > FILE_DESCRIPTOR_COUNT)
                return -1;

            process_t* process = fetch_process(pid);
            if (process->file_descriptors[fd] == (void*) 0)
                return -1;
            close_generic_file(process->file_descriptors[fd]);
            process->file_descriptors[fd] = (void*) 0;
            return 0;
        }

        // void* mmap(void* addr, unsigned long long length, int prot, int flags, int fd, unsigned long long offset);
        case 9: {
            // void* addr = (void*) a0;
            unsigned long long length = a1;
            int prot = (int) a2;
            // int flags = (int) a3;
            // int fd = (int) a4;
            // unsigned long long offset = a5;

#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_EXEC 4

            // Write+exec is illegal for security reasons
            if ((prot & PROT_WRITE) && (prot & PROT_EXEC))
                return -1;

            unsigned long long page_num = (length + PAGE_SIZE - 1) / PAGE_SIZE;
            void* alloced = alloc_page(page_num);
            if (alloced == (void*) 0)
                return -1;

            short f = 0;
            process_t* process = fetch_process(pid);
            if (prot & PROT_READ)
                f |= MMU_FLAG_READ;
            if (prot & PROT_WRITE)
                f |= MMU_FLAG_WRITE;
            if (prot & PROT_EXEC)
                f |= MMU_FLAG_EXEC;
            for (unsigned long long i = 0; i < page_num; i++) {
                mmu_protect(process->mmu_data, alloced + i * PAGE_SIZE, MMU_FLAG_USER | MMU_FLAG_ALLOCED | f, 1);
            }

            return (unsigned long long) alloced;
        }

        // int mprotect(void* addr, unsigned long long length, int prot);
        case 10: {
            if (a0 & 0xfff)
                return -1;

            void* addr = (void*) a0;
            unsigned long long size = a1;
            int prot = (int) a2;

            if ((prot & PROT_WRITE) && (prot & PROT_EXEC))
                return -1;

            unsigned long long page_num = (size + PAGE_SIZE - 1) / PAGE_SIZE;
            short f = 0;
            process_t* process = fetch_process(pid);
            if (prot & PROT_READ)
                f |= MMU_FLAG_READ;
            if (prot & PROT_WRITE)
                f |= MMU_FLAG_WRITE;
            if (prot & PROT_EXEC)
                f |= MMU_FLAG_EXEC;
            for (unsigned long long i = 0; i < page_num; i++) {
                int r = mmu_protect(process->mmu_data, addr + i * PAGE_SIZE, MMU_FLAG_USER | f, 1);
                if (r != 0)
                    return -1;
            }
            return 0;
        }

        // int munmap(void* addr, unsigned long long length);
        case 11: {
            void* addr = (void*) a0;
            unsigned long long size = a1;

            unsigned long long page_num = (size + PAGE_SIZE - 1) / PAGE_SIZE;
            process_t* process = fetch_process(pid);
            for (unsigned long long i = 0; i < page_num; i++) {
                unmap_mmu(process->mmu_data, addr);
            }
            return 0;
        }

        // pid_t getpid(void);
        case 39:
            return pid;

        // void exit(void* returned, unsigned long long length);
        case 60: {
            // TODO: use this value
            void* returned = (void*) a0;
            unsigned long long length = a1;

            kill_process(pid);
            sbi_set_timer(0);
            return 0;
        }

        // pid_t getppid(void);
        case 110:
            return fetch_process(pid)->parent_pid;

        // pid_t spawn(char* path, char* argv[], char* envp[], int stdin, int stdout, int stderr);
        case 314: {
            char* path = (void*) a0;

            // TODO: use these
            char** argv = (void*) a1;
            char** envp = (void*) a2;

            int stdin  = (int) a3;
            int stdout = (int) a4;
            int stderr = (int) a5;

            // Create process
            elf_t elf = load_executable_elf_from_file(root, path);
            pid_t p = load_elf_as_process(pid, &elf, 1);
            free_elf(&elf);
            process_init_kernel_mmu(p);

            // Set file descriptors
            process_t* process = fetch_process(pid);
            process_t* child = fetch_process(p);

            if (stdin < FILE_DESCRIPTOR_COUNT && process->file_descriptors[stdin] != (void*) 0) {
                child->file_descriptors[0] = malloc(sizeof(generic_file_t));
                copy_generic_file(child->file_descriptors[0], process->file_descriptors[stdin]);
            }

            if (stdout < FILE_DESCRIPTOR_COUNT && process->file_descriptors[stdout] != (void*) 0) {
                child->file_descriptors[1] = malloc(sizeof(generic_file_t));
                copy_generic_file(child->file_descriptors[1], process->file_descriptors[stdout]);
            }

            if (stderr < FILE_DESCRIPTOR_COUNT && process->file_descriptors[stderr] != (void*) 0) {
                child->file_descriptors[1] = malloc(sizeof(generic_file_t));
                copy_generic_file(child->file_descriptors[1], process->file_descriptors[stderr]);
            }

            // Add process to queue
            add_process_to_queue(p);
            return p;
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

