.set UART_BASE, 0x10000000

.section .text
.global uart_init
.global loader_uart_puts
.global loader_uart_getc


# uart_init(void) -> void
# Initialises the 16550a UART chip.
#
# Parameters: nothing
# Returns: nothing
uart_init:
    # QEMU automatically sets up the registers for us
    # Except possibly the interrupt register (idk if that is set up to no interrupts)
    # So we set up the interrupt register for it
    li t0, UART_BASE

    # M - Modem status interrupt
    # R - Receiver line status interrupt
    # T - Transmitter holding register empty interrupt
    # D - received Data available interrupt
    #            MRTD
    li t1, 0b00000000
    sw t1, 0x04(t0)

    ret


# loader_uart_puts(char*) -> void
# Puts a string onto the UART port.
#
# Parameters:
# a0: char*     - The pointer to the null terminated string to be printed out.
# Returns: nothing
loader_uart_puts:
    li t0, UART_BASE

puts_loop:
    lbu a1, 0x00(a0)
    beqz a1, puts_end

puts_wait:
    lw t1, 0x00(t0)
    bnez t1, puts_wait

    sw a1, 0x00(t0)

    addi a0, a0, 1
    j puts_loop

puts_end:
    ret


# loader_uart_getc(void) -> char
# Gets a character from the UART port and echos it back.
#
# Parameters: nothing
# Returns:
# a0: char      - The last character received.
loader_uart_getc:
    li t0, UART_BASE

getc_loop:
    lbu a0, 0x05(t0)
    andi a0, a0, 0b00000001
    beqz a0, getc_loop

    lw a0, 0x00(t0)
    sw a0, 0x00(t0)
    ret

