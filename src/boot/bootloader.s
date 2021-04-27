.section .text
.global _start


_start:
    # Use only the zeroth hart
    csrr t0, mhartid
    bnez t0, finish

    # Initialise UART
    jal uart_init

    # Put the message on the UART port
    la a0, msg
    jal uart_puts

    # Loop forever
finish:
    jal uart_getc
    j finish


.section .rodata
msg:
    .string "hewwo wowwd\r\n"
    .byte 0

