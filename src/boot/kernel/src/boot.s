.section .text
.global _start
.global interrupt_init

_start:
    # Initialise stack pointer and mscratch
    #la sp, isr_stack_end
    #csrrw sp, sscratch, sp
    la sp, stack_top
    mv fp, sp

    la a0, welcome_msg
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
    sfence.vma

    # Jump to kernel init
    la a0, kinit
    csrw sepc, a0
    la ra, interrupt_init
    li a0, 0x40100
    csrs sstatus, a0
    sret

interrupt_init:
    # Enable all interrupts in the PLIC
    li t0, 0xffffffff
    li t1, 0x0c002000
    sw t0, (t1)
    sw t0, 0x4(t1)

    /* This is for debugging purposes
    # Set interrupt priorities
    li t0, 0x7
    li t1, 0x0c000004
    li t2, 0x0c0000d9
interrupt_priority_set_loop:
    sw t0, (t1)
    addi t1, t1, 0x4
    blt t1, t2, interrupt_priority_set_loop
    #*/

    # Enable interrupts
    li t0, 0x200
    csrs sie, t0
    li t0, 0x2
    csrs sstatus, t0

    # Set priority threshold
    li t1, 0x0C200000
    sw zero, (t1)

    # Jump to kernel main
    jal kmain

finish:
    j finish


.section .rodata
welcome_msg:
    .string "                 _____ ___   
                (  _  )  _ \\ 
   _   _   _   _| ( ) | (_(_)
 / _ \\( ) ( ) ( ) | | |\\__ \\ 
( (_) ) \\_/ \\_/ | (_) | )_) |
 \\___/ \\__/\\___/(_____)\\____)
"
    .byte 0

