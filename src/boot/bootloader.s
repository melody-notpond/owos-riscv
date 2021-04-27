.section .text
.globl _start


_start:
    csrr t0, mhartid
    bnez t0, finish

    jal gpu_init

    la a0, msg
    jal puts

finish:
    j finish


.section .rodata
msg:
    .string "hewwo wowwd\r\n"
    .byte 0
