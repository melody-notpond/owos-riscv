.set UART_BASE, 0x10000000

.section .text
.global uart_init
.global uart_put_hex
.global uart_putc
.global uart_puts
.global uart_getc


# uart_init(void) -> void
# Initialises the 16550a UART chip.
#
# Parameters: nothing
# Returns: nothing
# Used registers:
# - t0
# - t1
uart_init:
    li t0, UART_BASE

    # Allow writing to the divisor latch registers
    li t1, 0b10000000
    sb t1, 0x03(t0)

    # Write baudrate divisor constant
    li t1, 0x6f
    sb t1, 0x00(t0)
    li t1, 0x00
    sb t1, 0x01(t0)

    # Set line control register
    # D  - Divisor latch access (disabled)
    # B  - force Break          (disabled)
    # F  - Force parity         (disabled)
    # E  - Even parity          (disabled)
    # P  - enable Parity        (enabled)
    # S  - Stop bits            (2 bits)
    # WW - Word size            (8 bits)
    #        DBFEPSWW
    li t1, 0b00001111
    sb t1, 0x03(t0)

    # Have no interrupts for now
    # M - Modem status interrupt                        (disabled)
    # R - Receiver line status interrupt                (disabled)
    # T - Transmitter holding register empty interrupt  (disabled)
    # D - received Data available interrupt             (disabled)
    #            MRTD
    li t1, 0b00000000
    sb t1, 0x01(t0)

    ret


# uart_putc(char) -> void
# Puts a character onto the UART port.
#
# Parameters:
# a1: char      - The character to be printed out.
# Returns: nothing
# Used registers:
# - a0
# - t0
# - t1
uart_putc:
    # Load UART base
    li t0, UART_BASE

    # Wait until UART is ready for the next character
uart_putc_wait:
    lbu t1, 0x05(t0)
    andi t1, t1, 0b01000000
    beqz t1, uart_putc_wait

    # Send character
    sw a0, 0x00(t0)
    ret


# putc_wait(char) -> void
# Puts a character onto the UART port. This is used by the assembly functions here since it takes in its argument in a1, saving time pushing onto the stack.
#
# Parameters:
# a1: char      - The character to be printed out.
# Returns: nothing
# Used registers:
# - a1
# - t0
# - t1
putc_wait:
    # Wait until UART is ready for the next character
    lbu t1, 0x05(t0)
    andi t1, t1, 0b01000000
    beqz t1, putc_wait

    # Send character
    sw a1, 0x00(t0)
    ret


# uart_puts(char*) -> void
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
uart_puts:
    # Push the return address
    addi sp, sp, -0x10
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
    addi sp, sp, 0x10
    ret


# uart_getc(void) -> char
# Gets a character from the UART port and echos it back.
#
# Parameters: nothing
# Returns:
# a0: char      - The last character received.
# Used registers:
# - a0
# - t0
# - t1
uart_getc:
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
uart_put_hex_array:
    .string "0123456789ABCDEF"
    .byte 0

