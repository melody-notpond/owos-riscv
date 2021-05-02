.section .text
.global _start


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

    # Set machine trap vector
    la t0, interrupt_handler
    csrw mtvec, t0

    # Enable interrupts
    li t0, 0b100000001000
    csrs mie, t0
    li t0, 0b00001000
    csrs mstatus, t0

    # Jump to kernel
    jal kinit
finish:
    j finish


.section .rodata
uart_init_msg:
    .string "Initialised UART\n"
    .byte 0

