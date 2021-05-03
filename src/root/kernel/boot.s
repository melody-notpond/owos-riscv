.section .text
.global _start
.global interrupt_init

_start:
    # Use only the zeroth hart
    csrr t0, mhartid
    bnez t0, finish

    # Initialise stack pointer and mscratch
    la sp, isr_stack_end
    csrrw sp, mscratch, sp
    la sp, stack_top
    mv fp, sp

    # Initialise UART
    jal uart_init

    # Tell user that UART is initialised
    la a0, uart_init_msg
    jal uart_puts

    # Jump to kernel init
    j kinit

interrupt_init:
    # Set machine trap vector
    la t0, interrupt_handler
    csrw mtvec, t0

    # Enable all interrupts in the PLIC
    li t0, 0xffffffff
    li t1, 0x0c002000
    sw t0, (t1)
    sw t0, 0x4(t1)

    #/* This is for debugging purposes
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
    li t0, 0x800
    csrs mie, t0
    li t0, 0x8
    csrs mstatus, t0

    # Set priority threshold
    li t1, 0x0C200000
    sw zero, (t1)

    # Jump to kernel main
    j kmain

finish:
    j finish


.section .rodata
uart_init_msg:
    .string "Initialised UART\n"
    .byte 0

