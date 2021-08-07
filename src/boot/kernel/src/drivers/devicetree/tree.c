#include "tree.h"
#include "../console/console.h"

// be_to_le(unsigned long long, unsigned char*) -> unsigned long long
// Converts a big endian number into a little endian number.
unsigned long long be_to_le(unsigned long long size, unsigned char* be) {
    unsigned long long byte_count = size / 8;
    unsigned long long result = 0;

    for (unsigned long long i = 0; i < byte_count; i++) {
        result |= ((unsigned long long) be[i]) << ((byte_count - i - 1) * 8);
    }

    return result;
}

// verify_fdt(void*) -> fdt_t
// Verifies a fdt by checking its magic number.
fdt_t verify_fdt(void* fdt) {
    fdt_header_t* header = fdt;
    unsigned long long magic = be_to_le(32, header->magic);
    if (be_to_le(32, header->magic) == 0xd00dfeed && be_to_le(32, header->version) == 17)
        return (fdt_t) {
            .header = header,
            .memory_reservation_block = fdt + be_to_le(32, header->off_mem_rsvmap),
            .structure_block = fdt + be_to_le(32, header->off_dt_struct),
            .strings_block = fdt + be_to_le(32, header->off_dt_strings)
        };

    return (fdt_t) { 0 };
}

// dump_fdt(fdt_t*) -> void
// Dumps an fdt to the UART.
void dump_fdt(fdt_t* fdt) {
    if (fdt->header == (void*) 0) {
        console_puts("Invalid flat device tree\n");
        return;
    }

    console_printf(
        "Header at %p:\n"
        "    magic: 0x%llx\n"
        "    total size: 0x%llx\n"
        "    structure offset: 0x%llx\n"
        "    strings offset: 0x%llx\n"
        "    memory reserved map offset: 0x%llx\n"
        "    version: 0x%llx\n"
        "    last compatible version: 0x%llx\n"
        "    boot cpu id: 0x%llx\n"
        "    strings size: 0x%llx\n"
        "    structure size: 0x%llx\n"
        , fdt->header
        , be_to_le(32, fdt->header->magic)
        , be_to_le(32, fdt->header->totalsize)
        , be_to_le(32, fdt->header->off_dt_struct)
        , be_to_le(32, fdt->header->off_dt_strings)
        , be_to_le(32, fdt->header->off_mem_rsvmap)
        , be_to_le(32, fdt->header->version)
        , be_to_le(32, fdt->header->last_comp_version)
        , be_to_le(32, fdt->header->boot_cpuid_phys)
        , be_to_le(32, fdt->header->size_dt_strings)
        , be_to_le(32, fdt->header->size_dt_struct)
    );

    struct fdt_reserve_entry* entry = fdt->memory_reservation_block;
    unsigned long long addr = be_to_le(64, entry->address);
    unsigned long long size = be_to_le(64, entry->size);
    console_puts("Memory reserved map:\n");
    while (addr || size) {
        console_printf("    reserved %llx-%llx (%llx bytes)\n", addr, addr + size, size);
        entry++;
        addr = be_to_le(64, entry->address);
        size = be_to_le(64, entry->size);
    }
    console_puts("End of memory reserved map\n");

    console_puts("Structure:\n");
    void* ptr = fdt->structure_block;
    int indent = 0;
    unsigned long long current;
    while ((current = be_to_le(32, ptr)) != FDT_END) {
        switch ((fdt_node_type_t) current) {
            case FDT_BEGIN_NODE: {
                for (int i = 0; i < indent; i++) {
                    console_puts("    ");
                }

                indent += 1;
                ptr += 4;
                char* c = ptr;
                if (indent == 1) {
                    console_puts("root:\n");
                } else {
                    while (*c) {
                        console_printf("%c", *c++);
                    }
                    console_puts(":\n");
                }

                ptr = (void*) ((unsigned long long) (c + 4) & ~0x3);
                break;
            }

            case FDT_END_NODE:
                indent -= 1;
                ptr += 4;
                break;

            case FDT_PROP:
                for (int i = 0; i < indent; i++) {
                    console_puts("    ");
                }

                ptr += 4;
                unsigned int len = be_to_le(32, ptr);
                ptr += 4;
                unsigned int name_offset = be_to_le(32, ptr);
                ptr += 4;

                console_printf("property %s = ", fdt->strings_block + name_offset);

                if (*((char*) ptr) == 0) {
                    console_printf("0x%llx\n", be_to_le(len * 8, ptr));
                } else {
                    for (unsigned int i = 0; i < len; i++) {
                        console_printf("%c", ((char*) ptr)[i]);
                    }
                    console_puts("\n");
                }

                ptr = (void*) ((unsigned long long) (ptr + len + 3) & ~0x3);
                break;

            case FDT_NOP:
                ptr += 4;
                break;

            case FDT_END:
                break;
        }
    }
}
