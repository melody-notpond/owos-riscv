.section .text

.global process_switch_context

# process_switch_context(process_t*) -> void
# Switches the current context to a process and continues execution of that process.
#
# Parameters:
# a0: process_t*    - The pid of the process to switch to.
# Returns: nothing
# Modifies all registers.
/*
typedef struct s_process {
    pid_t pid;
    pid_t parent_pid;
    process_state_t state;
    mmu_level_1_t* mmu_data;
    generic_file_t** file_descriptors;
    unsigned long long pc;
    unsigned long long xs[32];
    double fs[32];
} process_t;
*/
process_switch_context:
    # Set registers
    ld x0,  0x030(a0)
    ld x1,  0x038(a0)
    ld x2,  0x040(a0)
    ld x3,  0x048(a0)
    ld x4,  0x050(a0)
    ld x5,  0x058(a0)
    ld x6,  0x060(a0)
    ld x7,  0x068(a0)
    ld x8,  0x070(a0)
    ld x9,  0x078(a0)
    ld x13, 0x098(a0)
    ld x14, 0x0a0(a0)
    ld x15, 0x0a8(a0)
    ld x16, 0x0b0(a0)
    ld x17, 0x0b8(a0)
    ld x18, 0x0c0(a0)
    ld x19, 0x0c8(a0)
    ld x20, 0x0d0(a0)
    ld x21, 0x0d8(a0)
    ld x22, 0x0e0(a0)
    ld x23, 0x0e8(a0)
    ld x24, 0x0f0(a0)
    ld x25, 0x0f8(a0)
    ld x26, 0x100(a0)
    ld x27, 0x108(a0)
    ld x28, 0x110(a0)
    ld x29, 0x118(a0)
    ld x30, 0x120(a0)
    ld x31, 0x128(a0)

    # Set program counter
    ld a1, 0x028(a0)
    csrw sepc, a1

    # Set ring to user ring
    li a1, 0x100
    csrc sstatus, a1

    # Set sscratch to pid
    ld a1, 0x000(a0)
    csrw sscratch, a1

    # a0, a1, and a2 are done last
    ld a1,  0x088(a0)
    ld a2,  0x090(a0)
    ld a0,  0x080(a0)

    # Jump to process
    sret

