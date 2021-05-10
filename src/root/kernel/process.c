#include "memory.h"
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
            .pc = 0x80000000,
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
                .pc = 0x80000000,
                .xs = { 0 },
                .fs = { 0.0 }
            };
            return i;
        }
    }

    return -1;
}

