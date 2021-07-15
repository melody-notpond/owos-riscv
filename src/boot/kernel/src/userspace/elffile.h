#ifndef KERNEL_ELF_FILE_H
#define KERNEL_ELF_FILE_H

#include "../drivers/filesystems/generic_file.h"

enum {
    ELF_TYPE_NONE = 0,
    ELF_TYPE_RELOCATABLE,
    ELF_TYPE_EXECUTABLE,
    ELF_TYPE_DYNAMIC,
    ELF_TYPE_CORE,
    ELF_TYPE_LOW_PROC = 0xff00,
    ELF_TYPE_HIGH_PROC = 0xffff
};

typedef struct __attribute__((packed, aligned(2))) {
    // Identifier stuff
    unsigned char ident[16];

    // The type of the object file
    unsigned short type;

    // The machine architecture the elf file is fore
    unsigned short machine;

    // The version of the elf file (must be 1)
    unsigned int version;

    // The entry point of the program
    unsigned long long entry;

    // The program header offset from the beginning of the file, zero if nonexistent
    unsigned long long program_header_offset;

    // The section header offset from the beginning of the file, zero if nonexistent
    unsigned long long section_header_offset;

    // Machine architecture flags
    unsigned int flags;

    // The size of the elf header
    unsigned short elf_header_size;

    // The size of one program header entry
    unsigned short program_header_entry_size;

    // The number of program headers
    unsigned short program_header_num;

    // The size of a section header entry
    unsigned short section_header_entry_size;

    // The number of section headers
    unsigned short section_header_num;

    // The section header index associated with the string table
    unsigned short section_header_string_table_index;
} elf_header_t;

typedef struct {
    // The type of the program header
    unsigned int type;

    // Flags for the header
    unsigned int flags;

    // Where the segment is relative to the beginning of the file
    unsigned long long offset;

    unsigned long long virtual_address;
    unsigned long long physical_address;

    // The size of the segment on the file
    unsigned long long file_size;

    // The size of the segment in memory
    unsigned long long mem_size;

    // The alignment with which the segment must be placed in memory
    unsigned long long align;
} elf_program_header_t;

typedef struct {
    elf_header_t header;
    elf_program_header_t* program_headers;
    void** data;
} elf_t;

// load_executable_elf_from_file(generic_file_t*, char*) -> elf_t
// Loads an elf file from disk.
elf_t load_executable_elf_from_file(generic_file_t* dir, char* path);

// free_elf(elf_t*) -> void
// Frees memory associated with an elf structure.
void free_elf(elf_t* elf);

#endif /* KERNEL_ELF_FILE_H */

