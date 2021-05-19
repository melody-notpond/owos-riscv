#include "elffile.h"
#include "../lib/memory.h"

#define ELF_MACHINE_RISCV 0xf3
#define ELF_CLASS_64 2
#define ELF_DATA_LIL_ENDIAN 1

// load_executable_elf_from_file(generic_dir_t*, char*) -> elf_t
// Loads an header file from disk.
elf_t load_executable_elf_from_file(generic_dir_t* dir, char* path) {
    struct s_dir_entry entry = generic_dir_lookup(dir, path);
    if (entry.tag != DIR_ENTRY_TYPE_REGULAR)
        return (elf_t) { 0 };

    // When the impostor is sus!
    generic_file_t* file = entry.value.file;

    // Read header header
    elf_header_t header;
    generic_file_read(file, &header, sizeof(elf_header_t));

    // Check magic number
    if (!(header.ident[0] == 0x7f && header.ident[1] == 'E' && header.ident[2] == 'L' && header.ident[3] == 'F')) {
        close_generic_file(file);
        return (elf_t) { 0 };
    }

    // Check if 64 bit
    if (header.ident[4] != ELF_CLASS_64) {
        close_generic_file(file);
        return (elf_t) { 0 };
    }

    // Check endianness
    if (header.ident[5] != ELF_DATA_LIL_ENDIAN) {
        // TODO: support big endian header files
        close_generic_file(file);
        return (elf_t) { 0 };
    }

    // Check header version
    if (header.ident[6] != 1) {
        close_generic_file(file);
        return (elf_t) { 0 };
    }

    // Check if executable
    if (header.type != ELF_TYPE_EXECUTABLE) {
        close_generic_file(file);
        return (elf_t) { 0 };
    }

    // Check if for RISC V
    if (header.machine != ELF_MACHINE_RISCV) {
        close_generic_file(file);
        return (elf_t) { 0 };
    }

    // Check if the version is valid
    if (header.version != 1) {
        close_generic_file(file);
        return (elf_t) { 0 };
    }

    // Create elf file structure
    elf_t elf = { 0 };
    elf.header = header;
    elf.program_headers = malloc(header.program_header_num * sizeof(elf_program_header_t));

    // Seek to program header
    generic_file_seek(file, header.program_header_offset);

    // Load program headers
    unsigned long long offset = header.program_header_offset;
    for (int i = 0; i < header.program_header_num; i++) {
        generic_file_read(file, elf.program_headers + i, sizeof(elf_program_header_t));
        offset += header.program_header_entry_size;
        generic_file_seek(file, offset);
    }

    // Load program data
    elf.data = malloc(header.program_header_num * sizeof(void*));
    for (int i = 0; i < header.program_header_num; i++) {
        elf.data[i] = malloc(elf.program_headers[i].file_size);
        generic_file_seek(file, elf.program_headers[i].offset);
        generic_file_read(file, elf.data[i], elf.program_headers[i].file_size);
    }

    close_generic_file(file);

    return elf;
}

// free_elf(elf_t*) -> void
// Frees memory associated with an elf structure.
void free_elf(elf_t* elf) {
    for (int i = 0; i < elf->header.program_header_num; i++) {
        free(elf->data[i]);
    }
    free(elf->data);
    free(elf->program_headers);
}
