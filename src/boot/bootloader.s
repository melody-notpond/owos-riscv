.section .text
.globl _start


_start:
    csrr t0, mhartid
    bnez t0, finish

    la a0, msg
    jal puts

finish:
    j finish


.section .rodata
msg:
    .string "hewwo wowwd\n"
    .byte 0
