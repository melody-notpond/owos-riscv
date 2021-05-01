.section .text
.global interrupt_handler
.align 2


interrupt_handler:
    la a0, interrupt_msg
    jal uart_puts
hang:
    j hang


.section .rodata
interrupt_msg:
    .string "uwu\n"
    .byte 0

