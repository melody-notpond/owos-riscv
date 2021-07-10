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
    unsigned long long pc;
    unsigned long long xs[32];
    double fs[32];
} process_t;
*/
process_switch_context:
    # Set registers
    ld x0, 0x28(a0)
    ld x1, 0x30(a0)
    ld x2, 0x38(a0)
    ld x3, 0x40(a0)
    ld x4, 0x48(a0)
    ld x5, 0x50(a0)
    ld x6, 0x58(a0)
    ld x7, 0x60(a0)
    ld x8, 0x68(a0)
    ld x9, 0x70(a0)
    ld x13, 0x90(a0)
    ld x14, 0x98(a0)
    ld x15, 0xa0(a0)
    ld x16, 0xa8(a0)
    ld x17, 0xb0(a0)
    ld x18, 0xb8(a0)
    ld x19, 0xc0(a0)
    ld x20, 0xc8(a0)
    ld x21, 0xd0(a0)
    ld x22, 0xd8(a0)
    ld x23, 0xe0(a0)
    ld x24, 0xe8(a0)
    ld x25, 0xf0(a0)
    ld x26, 0xf8(a0)
    ld x27, 0x100(a0)
    ld x28, 0x108(a0)
    ld x29, 0x110(a0)
    ld x30, 0x118(a0)
    ld x31, 0x120(a0)

    # Set program counter
    ld a1, 0x20(a0)
    csrw sepc, a1

    # Get page table
    ld a1, 0x18(a0)
    srli a1, a1, 12
    li a2, 0x8000000000000000
    or a1, a2, a1
    csrw sscratch, a1

    # Set ring to user ring
    li a1, 0x100
    csrc sstatus, a1

    # a0, a1, and a2 are done last
    ld a1, 0x80(a0)
    ld a2, 0x88(a0)
    ld a0, 0x78(a0)

    # Set satp and jump to process
    csrrw t0, sscratch, t0
    csrw satp, t0
    mv t0, zero
    sfence.vma zero, t0
    csrrw t0, sscratch, t0
    sret

