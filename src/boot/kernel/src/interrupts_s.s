.section .text
.global interrupt_handler
.align 2

interrupt_handler:
    # Push registers
    #csrrw sp, sscratch, sp
    addi sp, sp, -0x200
    sd x0,  0x000(sp)
    sd x1,  0x008(sp)
    sd x2,  0x010(sp)
    sd x3,  0x018(sp)
    sd x4,  0x020(sp)
    sd x5,  0x028(sp)
    sd x6,  0x030(sp)
    sd x7,  0x038(sp)
    sd x8,  0x040(sp)
    sd x9,  0x048(sp)
    sd x10, 0x050(sp)
    sd x11, 0x058(sp)
    sd x12, 0x060(sp)
    sd x13, 0x068(sp)
    sd x14, 0x070(sp)
    sd x15, 0x078(sp)
    sd x16, 0x080(sp)
    sd x17, 0x088(sp)
    sd x18, 0x090(sp)
    sd x19, 0x098(sp)
    sd x20, 0x0a0(sp)
    sd x21, 0x0a8(sp)
    sd x22, 0x0b0(sp)
    sd x23, 0x0b8(sp)
    sd x24, 0x0c0(sp)
    sd x25, 0x0c8(sp)
    sd x26, 0x0d0(sp)
    sd x27, 0x0d8(sp)
    sd x28, 0x0e0(sp)
    sd x29, 0x0e8(sp)
    sd x30, 0x0f0(sp)
    sd x31, 0x0f8(sp)

    # Call interrupt handler
    csrr a0, scause
    csrr a1, sepc
    mv a2, sp
    jal handle_interrupt
    csrw sepc, a0

    # Pop registers
    ld x1, 0x08(sp)
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
    ld x16, 0x80(sp)
    ld x17, 0x88(sp)
    ld x18, 0x90(sp)
    ld x19, 0x98(sp)
    ld x20, 0xa0(sp)
    ld x21, 0xa8(sp)
    ld x22, 0xb0(sp)
    ld x23, 0xb8(sp)
    ld x24, 0xc0(sp)
    ld x25, 0xc8(sp)
    ld x26, 0xd0(sp)
    ld x27, 0xd8(sp)
    ld x28, 0xe0(sp)
    ld x29, 0xe8(sp)
    ld x30, 0xf0(sp)
    ld x31, 0xf8(sp)
    addi sp, sp, 0x200
    #csrrw sp, sscratch, sp
    sret


# Stack stuff
/*
.section .bss
.align 4
isr_stack_start:
    .skip 8192
isr_stack_end:
.global isr_stack_end
*/
