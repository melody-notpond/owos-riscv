.section .text
.global interrupt_handler
.align 2


interrupt_handler:
    # csrrw sp, mscratch, sp
    addi sp, sp, -0x80
    sd x1, 0x08(sp)
    sd x2, 0x10(sp)
    sd x3, 0x18(sp)
    sd x4, 0x20(sp)
    sd x5, 0x28(sp)
    sd x6, 0x30(sp)
    sd x7, 0x38(sp)
    sd x8, 0x40(sp)
    sd x9, 0x48(sp)
    sd x10, 0x50(sp)
    sd x11, 0x58(sp)
    sd x12, 0x60(sp)
    sd x13, 0x68(sp)
    sd x14, 0x70(sp)
    sd x15, 0x78(sp)

    #/*
    # Print out debug info
    la a0, interrupt_msg
    jal uart_puts
    csrr a0, mcause
    jal uart_put_hex
    la a0, '\n'
    jal uart_putc
    #*/

    ld x1, 0x08(sp)
    ld x2, 0x10(sp)
    ld x3, 0x18(sp)
    ld x4, 0x20(sp)
    ld x5, 0x28(sp)
    ld x6, 0x30(sp)
    ld x7, 0x38(sp)
    ld x8, 0x40(sp)
    ld x9, 0x48(sp)
    ld x10, 0x50(sp)
    ld x11, 0x58(sp)
    ld x12, 0x60(sp)
    ld x13, 0x68(sp)
    ld x14, 0x70(sp)
    ld x15, 0x78(sp)
    addi sp, sp, 0x80
    mret


.section .rodata
interrupt_msg:
    .string "Received interrupt for 0x"
    .byte 0

