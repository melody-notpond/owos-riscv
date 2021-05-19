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
    unsigned char ident[16];
    unsigned short type;
    unsigned short machine;
    unsigned int version;
    unsigned int entry;
    unsigned int program_header_offset;
    unsigned int section_header_offset;
    unsigned int flags;
    unsigned short elf_header_size;
    unsigned short program_header_entry_size;
    unsigned short program_header_num;
    unsigned short section_header_entry_size;
    unsigned short section_header_num;
    unsigned short section_header_string_table_index;
} elf_header_t;

typedef struct __attribute__((packed, aligned(2))) {
} elf_section_header_table;

// load_executable_elf_from_file(generic_dir_t*, char*) -> void
// Loads an elf file from disk.
void load_executable_elf_from_file(generic_dir_t* dir, char* path);

#endif /* KERNEL_ELF_FILE_H */

