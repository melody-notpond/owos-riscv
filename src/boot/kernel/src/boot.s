.section .text
.global _start
.global interrupt_init

# a0 - current hart id
# a1 - pointer to flattened device tree
_start:
    # Initialise stack pointer
    la sp, stack_top
    mv fp, sp

    mv t5, a0
    mv t6, a1

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
    jal init_heap_metadata
    jal create_mmu_top
    csrw sscratch, a0
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
    mv a0, t5
    mv a1, t6
    sret

finish:
    j finish


.section .rodata
hart_init_message:
    .string "Initialised hart 0x%llx\n"

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

