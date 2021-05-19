#include "elffile.h"
#include "../drivers/uart/uart.h"

#define ELF_MACHINE_RISCV 0xf3
#define ELF_CLASS_64 2
#define ELF_DATA_LIL_ENDIAN 1

// load_executable_elf_from_file(generic_dir_t*, char*) -> void
// Loads an elf file from disk.
void load_executable_elf_from_file(generic_dir_t* dir, char* path) {
    struct s_dir_entry entry = generic_dir_lookup(dir, path);
    if (entry.tag != DIR_ENTRY_TYPE_REGULAR)
        return;

    // When the impostor is sus!
    generic_file_t* file = entry.value.file;
    uart_printf("Found %s\n", path);

    // Read elf header
    elf_header_t elf;
    generic_file_read(file, &elf, sizeof(elf));

    // Check magic number
    if (!(elf.ident[0] == 0x7f && elf.ident[1] == 'E' && elf.ident[2] == 'L' && elf.ident[3] == 'F')) {
        uart_puts("Elf magic number not found\n");
        close_generic_file(file);
        return;
    }
    uart_puts("Elf magic number found!\n");

    // Check if 64 bit
    if (elf.ident[4] != ELF_CLASS_64) {
        uart_puts("Elf is not 64 bit\n");
        close_generic_file(file);
        return;
    }
    uart_puts("Elf file is 64 bit\n");

    // Check endianness
    if (elf.ident[5] != ELF_DATA_LIL_ENDIAN) {
        // TODO: support big endian elf files
        uart_puts("Big endian elf files are currently unsupported\n");
        close_generic_file(file);
        return;
    }

    // Check elf version
    if (elf.ident[6] != 1) {
        uart_puts("Elf version is invalid");
        close_generic_file(file);
        return;
    }

    // Check if executable
    if (elf.type != ELF_TYPE_EXECUTABLE) {
        uart_puts("Elf file is not executable\n");
        close_generic_file(file);
        return;
    }
    uart_puts("Elf is executable\n");

    // Check if for RISC V
    if (elf.machine != ELF_MACHINE_RISCV) {
        uart_puts("Elf file is for an architecture other than RISC V\n");
        close_generic_file(file);
        return;
    }
    uart_puts("Elf is for RISC V\n");

    // Check if the version is valid
    if (elf.version != 1) {
        uart_puts("Elf file is of invalid version\n");
        close_generic_file(file);
        return;
    }
    uart_puts("Elf is version 1\n");

    

    close_generic_file(file);
}

