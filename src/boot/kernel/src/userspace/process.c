#include "../lib/memory.h"
#include "elffile.h"
#include "process.h"

pid_t MAX_PID = 10000;
pid_t current_pid = 0;
process_t* process_table;

// init_process_table() -> void
// Initialises the process table.
void init_process_table() {
    unsigned long long page_num = (MAX_PID * sizeof(process_t) + PAGE_SIZE - 1) / PAGE_SIZE;
    process_table = alloc_page(page_num);
    MAX_PID = page_num * PAGE_SIZE / sizeof(process_t);
}

// spawn_process(pid_t) -> pid_t
// Spawns a process given its parent process. Returns -1 if unsuccessful.
pid_t spawn_process(pid_t parent_pid) {
    if (current_pid < MAX_PID) {
        process_table[current_pid] = (process_t) {
            .pid = current_pid,
            .parent_pid = parent_pid,
            .state = PROCESS_STATE_WAIT,
            .mmu_data = (void*) 0,
            .pc = 0,
            .xs = { 0 },
            .fs = { 0.0 }
        };
        pid_t pid = current_pid;
        current_pid++;
        return pid;
    }

    for (pid_t i = 0; i < MAX_PID; i++) {
        if (process_table[i].state == PROCESS_STATE_DEAD) {
            process_table[current_pid] = (process_t) {
                .pid = i,
                .parent_pid = parent_pid,
                .state = PROCESS_STATE_WAIT,
                .mmu_data = (void*) 0,
                .pc = 0,
                .xs = { 0 },
                .fs = { 0.0 }
            };
            return i;
        }
    }

    return -1;
}

// fetch_process(pid_t) -> process_t*
// Fetches a process from the process table.
process_t* fetch_process(pid_t pid) {
    return &process_table[pid];
}

// load_elf_as_process(pid_t, elf_t*) -> pid_t
// Uses an elf file as a process.
pid_t load_elf_as_process(pid_t parent_pid, elf_t* elf, unsigned int stack_page_count) {
    pid_t pid = spawn_process(parent_pid);
    process_t* process = fetch_process(pid);
    process->mmu_data = create_mmu_top();

    void* last_pointer = 0;
    for (int i = 0; i < elf->header.program_header_num; i++) {
        int j;
        void* ptr = (void*) elf->program_headers[i].virtual_address;
        for (j = 0; j < elf->program_headers[i].file_size; j += MMU_PAGE_SIZE) {
            void* page = alloc_page_mmu(process->mmu_data, ptr, MMU_FLAG_EXEC | MMU_FLAG_READ);
            unsigned long long size = elf->program_headers[i].file_size - j;
            memcpy(page, elf->data[i] + j, size < MMU_PAGE_SIZE ? size : MMU_PAGE_SIZE);
            ptr += MMU_PAGE_SIZE;
        }

        if (last_pointer < ptr)
            last_pointer = ptr;
    }

    for (unsigned int i = 0; i < stack_page_count; i++) {
        alloc_page_mmu(process->mmu_data, last_pointer, MMU_FLAG_READ | MMU_FLAG_WRITE);
        last_pointer += MMU_PAGE_SIZE;
    }

    process->pc = elf->header.entry;
    process->xs[PROCESS_REGISTER_SP] = (unsigned long long) last_pointer;
    process->xs[PROCESS_REGISTER_FP] = (unsigned long long) last_pointer;

    return pid;
}

