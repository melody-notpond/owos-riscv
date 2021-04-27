.set UART_BASE, 0x10000000

.section .text
.globl puts


puts:
    li t0, UART_BASE

puts_loop:
    lbu a1, (a0)
    beqz a1, puts_end
    sw a1, (t0)
    addi a0, a0, 1
    j puts_loop

puts_end:
    ret

