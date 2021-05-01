/*
static const MemMapEntry virt_memmap[] = {
    [VIRT_DEBUG] =       {        0x0,         0x100 },
    [VIRT_MROM] =        {     0x1000,        0xf000 },
    [VIRT_TEST] =        {   0x100000,        0x1000 },
    [VIRT_RTC] =         {   0x101000,        0x1000 },
    [VIRT_CLINT] =       {  0x2000000,       0x10000 },
    [VIRT_PCIE_PIO] =    {  0x3000000,       0x10000 },
    [VIRT_PLIC] =        {  0xc000000, VIRT_PLIC_SIZE(VIRT_CPUS_MAX * 2) },
    [VIRT_UART0] =       { 0x10000000,         0x100 },
    [VIRT_VIRTIO] =      { 0x10001000,        0x1000 },
    [VIRT_FW_CFG] =      { 0x10100000,          0x18 },
    [VIRT_FLASH] =       { 0x20000000,     0x4000000 },
    [VIRT_PCIE_ECAM] =   { 0x30000000,    0x10000000 },
    [VIRT_PCIE_MMIO] =   { 0x40000000,    0x40000000 },
    [VIRT_DRAM] =        { 0x80000000,           0x0 },
};
*/
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


# uart_put_hex(long long) -> void
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
uart_put_hex:
    # Push the return address
    addi sp, sp, -0x10
    sd ra, 0x0(sp)

    # Print out zero if it's zero
    bnez a0, uart_put_hex_nonzero
    li a1, '0'
    jal putc_wait
    j uart_put_hex_return

uart_put_hex_nonzero:
    # Initialise t0 and t1
    mv t0, a0
    li t1, -4

    # Get log_16(a0)
uart_put_hex_log_loop:
    srli t0, t0, 4
    addi t1, t1, 4
    bnez t0, uart_put_hex_log_loop

    # Initialise t0, t2, and t3
    li t0, UART_BASE
    li t2, 0b1111
    sll t2, t2, t1
    mv t3, t1

uart_put_hex_putc_loop:
    # Get character and print
    and a1, a0, t2
    srl a1, a1, t3
    la t4, uart_put_hex_array
    add a1, a1, t4
    lbu a1, 0x0(a1)
    jal putc_wait

    # Decrement and shift and branch
    addi t3, t3, -4
    srli t2, t2, 4
    bnez t2, uart_put_hex_putc_loop

uart_put_hex_return:
    ld ra, 0x0(sp)
    addi sp, sp, 0x10
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

