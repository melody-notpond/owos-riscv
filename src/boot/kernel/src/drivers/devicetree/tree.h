#ifndef DEVICE_TREE_H
#define DEVICE_TREE_H

typedef unsigned char be32_t[4];

typedef struct fdt_header {
    be32_t magic;
    be32_t totalsize;
    be32_t off_dt_struct;
    be32_t off_dt_strings;
    be32_t off_mem_rsvmap;
    be32_t version;
    be32_t last_comp_version;
    be32_t boot_cpuid_phys;
    be32_t size_dt_strings;
    be32_t size_dt_struct;
} __attribute__((packed, aligned(1))) fdt_header_t;

// be_to_le(unsigned long long, unsigned char*) -> unsigned long long
// Converts a big endian number into a little endian number.
unsigned long long be_to_le(unsigned long long size, unsigned char* be);

// verify_fdt(void*) -> fdt_header_t*
// Verifies a fdt by checking its magic number.
fdt_header_t* verify_fdt(void* fdt);

#endif /* DEVICE_TREE_H */
