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

.set SD_CARD_BASE, 0x10050000

.section .text
.global ext2fs_load_kernel


# ext2fs_load_kernel(void) -> void
# Loads the kernel from an ext2 file system.
#
# Parameters: nothing
# Returns: nothing
ext2fs_load_kernel:
    # Push the return address
    addi sp, sp, -0x8
    sd ra, 0x0(sp)

    # Tell user that SD is being loaded and booted into
    la a0, accessing_msg
    jal loader_uart_puts

    # Seek to magic number
    li a0, 0x0000000000000438
    jal ext2fs_sdcard_seek

    # Read magic number
    jal ext2fs_sdcard_read

    # Compare magic number
    bne t0, t1, ext2fs_return

    # Print message saying the file system magic number was verified
    la a0, ext2_magic_verified
    jal loader_uart_puts

    # Return
ext2fs_return:
    ld ra, 0x0(sp)
    addi sp, sp, 0x8
    ret


# ext2fs_sdcard_seek(long long) -> void
# Seeks to a memory address in the SD card.
#
# Parameters:
# a0: long long - The address to seek to.
# Returns: nothing
ext2fs_sdcard_seek:
    li t0, SD_CARD_BASE

    li t1, 0x03
    sw t1, 0x00(t0)
    ret


ext2fs_sdcard_read:
    ret


.section .rodata
accessing_msg:
    .string "Accessing SD card...\n"
    .byte 0

ext2_magic_verified:
    .string "ext2 file system verified.\n"
    .byte 0
