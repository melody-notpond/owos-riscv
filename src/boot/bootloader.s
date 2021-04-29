.section .text
.global _start


_start:
    # Use only the zeroth hart
    csrr t0, mhartid
    bnez t0, finish

    # Initialise stack pointer
    li sp, 0xc0000000

    # Initialise UART
    jal uart_init

    # Tell user that UART is initialised
    la a0, uart_init_msg
    jal loader_uart_puts

    # Boot into the hard drive
    jal ext2fs_load_kernel

    # If we are here, we failed to boot into the kernel
    la a0, loading_kernel_fail_msg
    jal loader_uart_puts

finish:
    j finish


.section .rodata
uart_init_msg:
    .string "Initialised UART\n"
    .byte 0

loading_kernel_fail_msg:
    .string "Error loading kernel: unknown error\nHalting.\n"
    .byte 0

