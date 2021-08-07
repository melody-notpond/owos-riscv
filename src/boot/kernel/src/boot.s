.section .text
.global _start

# a0 - current hart id
# a1 - pointer to flattened device tree
_start:
    # Initialise stack pointer
    la sp, stack_top
    mv fp, sp

    # Save arguments
    addi sp, sp, -16
    sd a0, 8(sp)
    sd a1, 0(sp)

    # Print out welcome message
    la a0, welcome_msg0
    jal console_puts
    la a0, welcome_msg1
    jal console_puts
    la a0, welcome_msg2
    jal console_puts
    la a0, welcome_msg3
    jal console_puts
    la a0, welcome_msg4
    jal console_puts
    la a0, welcome_msg5
    jal console_puts

    # Set supervisor trap vector
    la t0, interrupt_handler
    csrw stvec, t0

    # Set up mmu
    ld a0, 0(sp)
    jal init_heap_metadata
    jal create_mmu_top
    csrw sscratch, a0
    ld a1, 0(sp)
    jal mmu_map_kernel
    csrr a0, sscratch

    # Enable the mmu
    li t0, 0x8000000000000000
    srli a0, a0, 12
    or a0, a0, t0
    csrw satp, a0
    sfence.vma zero, zero

    # Jump to kernel init
    la a0, kinit
    csrw sepc, a0
    la ra, kmain
    li a0, 0x40122
    csrs sstatus, a0

    # Load arguments
    ld a1, 0(sp)
    ld a0, 8(sp)
    addi sp, sp, 16
    sret

finish:
    j finish


.section .rodata
welcome_msg0:
    .string "                 _____ ___   \n"
welcome_msg1:
    .string "                (  _  )  _ \\\n"
welcome_msg2:
    .string "   _   _   _   _| ( ) | (_(_)\n"
welcome_msg3:
    .string " / _ \\( ) ( ) ( ) | | |\\__ \\ \n"
welcome_msg4:
    .string "( (_) ) \\_/ \\_/ | (_) | )_) |\n"
welcome_msg5:
    .string " \\___/ \\__/\\___/(_____)\\____)\n"

