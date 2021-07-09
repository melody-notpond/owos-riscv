.section .text

.global sbi_console_putchar
.global sbi_console_getchar

# sbi_console_putchar(char) -> void
# Puts a character onto the UART port.
#
# Parameters:
# a1: char      - The character to be printed out.
# Returns: nothing
# a0 - Error
# a1 - Value
# Used registers:
# - a0
# - a6
# - a7
sbi_console_putchar:
    li a6, 0
    li a7, 1
    ecall
    ret

# sbi_console_getchar() -> int
# Puts a character onto the UART port.
#
# Parameters: nothing
# Returns:
# a0 - Error
# a1 - The charater that was received.
# Used registers:
# - a0
# - a6
# - a7
sbi_console_getchar:
    li a6, 0
    li a7, 2
    ecall
    ret
