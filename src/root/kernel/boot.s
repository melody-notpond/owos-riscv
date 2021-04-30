.section .text
.global _start


_start:
    # Use only the zeroth hart
    csrr t0, mhartid
    bnez t0, finish

    # Initialise stack pointer
    la sp, stack_top
    mv fp, sp

    # Initialise UART
    jal uart_init

    # Tell user that UART is initialised
    la a0, uart_init_msg
    jal uart_puts

    # Jump to kernel
    jal kinit
finish:
    j finish


.section .rodata
uart_init_msg:
    .string "Initialised UART\n"
    .byte 0

