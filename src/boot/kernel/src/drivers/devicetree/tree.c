#include "tree.h"

// be_to_le(unsigned long long, unsigned char*) -> unsigned long long
// Converts a big endian number into a little endian number.
unsigned long long be_to_le(unsigned long long size, unsigned char* be) {
    unsigned long long byte_count = size / 8;
    unsigned long long result = 0;

    for (unsigned long long i = 0; i < byte_count; i++) {
        result |= be[i] << ((byte_count - i - 1) * 8);
    }

    return result;
}

// verify_fdt(void*) -> fdt_header_t*
// Verifies a fdt by checking its magic number.
fdt_header_t* verify_fdt(void* fdt) {
    fdt_header_t* header = fdt;
    unsigned long long magic = be_to_le(32, header->magic);
    console_printf("magic: %llx\n", magic);
    if (be_to_le(32, header->magic) == 0xd00dfeed)
        return header;
    return (void*) 0;
}
