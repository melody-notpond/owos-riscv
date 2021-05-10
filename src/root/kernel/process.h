#ifndef KERNEL_PROCESS_H
#define KERNEL_PROCESS_H

#include "mmu.h"

typedef enum {
    PROCESS_STATE_DEAD,
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_WAIT,
    PROCESS_STATE_BLOCK
} process_state_t;

typedef unsigned long long pid_t;

typedef struct s_process {
    pid_t pid;
    pid_t parent_pid;
    process_state_t state;
    mmu_level_1_t* mmu_data;
    unsigned long long pc;
    unsigned long long xs[32];
    double fs[32];
} process_t;

// init_process_table() -> void
// Initialises the process table.
void init_process_table();

// spawn_process(pid_t) -> pid_t
// Spawns a process given its parent process. Returns -1 if unsuccessful.
pid_t spawn_process(pid_t parent_pid);

#endif /* KERNEL_PROCESS_H */

