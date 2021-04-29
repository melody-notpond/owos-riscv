.set UART_BASE, 0x10000000

.section .text
.global uart_init
.global loader_uart_put_hex
.global loader_uart_putc
.global loader_uart_puts
.global loader_uart_getc


# uart_init(void) -> void
# Initialises the 16550a UART chip.
#
# Parameters: nothing
# Returns: nothing
# Used registers:
# - t0
# - t1
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
    sw t1, 0x01(t0)

    ret


# loader_uart_put_hex(long long) -> void
# Puts a long long as a hex number into the UART port.
#
# Parameters:
# a0: long long     - The number to print out as hex.
# Returns: nothing
# Used registers:
# - a0
# - a1
# - t0
# - t1
# - t2
# - t3
# - t4
loader_uart_put_hex:
    # Push the return address
    addi sp, sp, -0x8
    sd ra, 0x0(sp)

    # Print out zero if it's zero
    bnez a0, loader_uart_put_hex_nonzero
    li a1, '0'
    jal loader_uart_putc
    j loader_uart_put_hex_return

loader_uart_put_hex_nonzero:
    # Initialise t0 and t1
    mv t0, a0
    li t1, -4

    # Get log_16(a0)
loader_uart_put_hex_log_loop:
    srli t0, t0, 4
    addi t1, t1, 4
    bnez t0, loader_uart_put_hex_log_loop

    # Initialise t0, t2, and t3
    li t0, UART_BASE
    li t2, 0b1111
    sll t2, t2, t1
    mv t3, t1

loader_uart_put_hex_putc_loop:
    # Get character and print
    and a1, a0, t2
    srl a1, a1, t3
    la t4, loader_uart_put_hex_array
    add a1, a1, t4
    lbu a1, 0x0(a1)
    jal putc_wait

    # Decrement and shift and branch
    addi t3, t3, -4
    srli t2, t2, 4
    bnez t2, loader_uart_put_hex_putc_loop

loader_uart_put_hex_return:
    ld ra, 0x0(sp)
    addi sp, sp, 0x8
    ret


# loader_uart_putc(char) -> void
# Puts a character onto the UART port.
#
# Parameters:
# a1: char      - The character to be printed out.
# Returns: nothing
# Used registers:
# - a1
# - t0
# - t1
loader_uart_putc:
    # Load UART base
    li t0, UART_BASE

    # Wait until UART is ready for the next character
putc_wait:
    lbu t1, 0x05(t0)
    andi t1, t1, 0b01000000
    beqz t1, putc_wait

    # Send character
    sw a1, 0x00(t0)
    ret


# loader_uart_puts(char*) -> void
# Puts a string onto the UART port.
#
# Parameters:
# a0: char*     - The pointer to the null terminated string to be printed out.
# Returns: nothing
# Used registers:
# - a0
# - a1
# - t0
# - t1
loader_uart_puts:
    # Push the return address
    addi sp, sp, -0x8
    sd ra, 0x0(sp)

    # Load the UART base address
    li t0, UART_BASE

    # Get next character, breaking if it is the null terminator
puts_loop:
    lbu a1, 0x00(a0)
    beqz a1, puts_end

    # Print character
    jal putc_wait

    # Increment pointer
    addi a0, a0, 1
    j puts_loop

    # Return
puts_end:
    ld ra, 0x0(sp)
    addi sp, sp, 0x8
    ret


# loader_uart_getc(void) -> char
# Gets a character from the UART port and echos it back.
#
# Parameters: nothing
# Returns:
# a0: char      - The last character received.
# Used registers:
# - a0
# - t0
# - t1
loader_uart_getc:
    li t0, UART_BASE

    # Wait until UART has a new character
getc_loop:
    lbu a0, 0x05(t0)
    andi a0, a0, 0b00000001
    beqz a0, getc_loop

    # Get character
    lw a0, 0x00(t0)

    # Wait until transmitter is ready
getc_echo_wait:
    lbu t1, 0x05(t0)
    andi t1, t1, 0b01000000
    beqz t1, getc_echo_wait

    sw a0, 0x00(t0)
    ret

.section .rodata
loader_uart_put_hex_array:
    .string "0123456789ABCDEF"
    .byte 0

