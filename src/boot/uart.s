/*

static const MemMapEntry sifive_u_memmap[] = {
    [SIFIVE_U_DEV_DEBUG] =    {        0x0,      0x100 },
    [SIFIVE_U_DEV_MROM] =     {     0x1000,     0xf000 },
    [SIFIVE_U_DEV_CLINT] =    {  0x2000000,    0x10000 },
    [SIFIVE_U_DEV_L2CC] =     {  0x2010000,     0x1000 },
    [SIFIVE_U_DEV_PDMA] =     {  0x3000000,   0x100000 },
    [SIFIVE_U_DEV_L2LIM] =    {  0x8000000,  0x2000000 },
    [SIFIVE_U_DEV_PLIC] =     {  0xc000000,  0x4000000 },
    [SIFIVE_U_DEV_PRCI] =     { 0x10000000,     0x1000 },
    [SIFIVE_U_DEV_UART0] =    { 0x10010000,     0x1000 },
    [SIFIVE_U_DEV_UART1] =    { 0x10011000,     0x1000 },
    [SIFIVE_U_DEV_QSPI0] =    { 0x10040000,     0x1000 },
    [SIFIVE_U_DEV_QSPI2] =    { 0x10050000,     0x1000 },
    [SIFIVE_U_DEV_GPIO] =     { 0x10060000,     0x1000 },
    [SIFIVE_U_DEV_OTP] =      { 0x10070000,     0x1000 },
    [SIFIVE_U_DEV_GEM] =      { 0x10090000,     0x2000 },
    [SIFIVE_U_DEV_GEM_MGMT] = { 0x100a0000,     0x1000 },
    [SIFIVE_U_DEV_DMC] =      { 0x100b0000,    0x10000 },
    [SIFIVE_U_DEV_FLASH0] =   { 0x20000000, 0x10000000 },
    [SIFIVE_U_DEV_DRAM] =     { 0x80000000,        0x0 },
};
*/

.set UART_BASE, 0x10010000

.section .text
.global uart_init
.global uart_puts
.global uart_getc


# uart_init(void) -> void
# Initialises the 16550a UART chip.
uart_init:
    # QEMU automatically sets up the registers for us
    # Except possibly the interrupt register (idk if that is set up to no interrupts)
    # So we set up the interrupt register for it
    li t0, UART_BASE

    li t1, 0b00000000
    sw t1, 0x10(t0)

    ret


# uart_puts(char*) -> void
# Puts a string onto the UART port.
# a0: char*     - The pointer to the null terminated string to be printed out.
# Returns: nothing
uart_puts:
    li t0, UART_BASE

puts_loop:
    lbu a1, 0x00(a0)
    beqz a1, puts_end

    # TODO: timings for hardware
puts_wait:
    lw t1, 0x00(t0)
    bnez t1, puts_wait

    sw a1, 0x00(t0)

    addi a0, a0, 1
    j puts_loop

puts_end:
    ret


# uart_getc(void) -> char
# Gets a character from the UART port.
# Returns:
# a0: char      - The last character received.
uart_getc:
    li t0, UART_BASE

getc_loop:
    lw a0, 0x04(t0)
    beqz a0, getc_loop

    sw a0, 0x00(t0)
    sw zero, 0x04(t0)
    ret

