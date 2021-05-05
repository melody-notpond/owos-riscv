#include "ext2.h"
#include "../memory.h"
#include "../string.h"
#include "../uart.h"

#define INODE_DIRECT_COUNT 12
#define INODE_SINGLE_INDIRECT 12
#define INODE_DOUBLE_INDIRECT 13
#define INODE_TRIPLE_INDIRECT 14

// ext2_load_superblock(generic_block_t*) -> ext2fs_superblock_t*
// Loads a superblock from a block device.
ext2fs_superblock_t* ext2_load_superblock(generic_block_t* block) {
    // Allocate the superblock in memory
    ext2fs_superblock_t* superblock = malloc(sizeof(ext2fs_superblock_t));

    // Read the superblock
    generic_block_read(block, superblock, 2, (sizeof(ext2fs_superblock_t) + SECTOR_SIZE - 1) / SECTOR_SIZE);

    // Check the magic number
    if (superblock->magic != 0xef53) {
        uart_puts("ext2 filesystem not found.\n");
        free(superblock);
        return (ext2fs_superblock_t*) 0;
    }

    // Debug info
    uart_puts("File system has 0x");
    uart_put_hex(superblock->inodes_count);
    uart_puts(" inodes.\n");

    uart_puts("File system has 0x");
    uart_put_hex(superblock->blocks_count);
    uart_puts(" blocks.\n");

    return superblock;
}

// ext2fs_load_block(generic_block_t*, ext2fs_superblock_t*, unsigned int) -> void*
// Loads a block from an ext2 file system.
void* ext2fs_load_block(generic_block_t* block, ext2fs_superblock_t* superblock, unsigned int block_id) {
    // Allocate the block in memory
    unsigned long long block_size = 1024 << superblock->log_block_size;
    void* data = malloc(block_size);

    // Read the block
    unsigned long long sector = block_id * block_size / SECTOR_SIZE;
    unsigned long long sector_count = block_size / SECTOR_SIZE;
    generic_block_read(block, data, sector, sector_count);

    return data;
}

// ext2_load_block_descriptor_table(generic_block_t*, ext2fs_superblock_t*) -> ext2fs_block_descriptor_t*
// Loads a descriptor table from an ext2 file system.
ext2fs_block_descriptor_t* ext2_load_block_descriptor_table(generic_block_t* block, ext2fs_superblock_t* superblock) {
    // Allocate the table
    unsigned int table_size = (superblock->blocks_count + superblock->blocks_per_group - 1) / superblock->blocks_per_group * sizeof(ext2fs_block_descriptor_t);
    ext2fs_block_descriptor_t* block_descriptor_table = malloc(table_size);

    // Read the table
    unsigned int sector_count = (table_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
    generic_block_read(block, block_descriptor_table, (1024 << superblock->log_block_size) / SECTOR_SIZE, sector_count);

    return block_descriptor_table;
}

ext2fs_inode_t* ext2_load_inode(generic_block_t* block, ext2fs_superblock_t* superblock, ext2fs_block_descriptor_t* desc_table, unsigned int inode) {
    // Inodes are one indexed because uhhhhh idk whyyyyyyyyyy :(
    inode--;

    // Allocate the inode and load the block in memory
    ext2fs_inode_t* root = malloc(superblock->inode_size);
    unsigned int desc_table_index = inode / superblock->inodes_per_group;
    unsigned int inode_table_offset = inode % superblock->inodes_per_group;
    unsigned long long block_size = 1024 << superblock->log_block_size;
    unsigned long long block_id = (desc_table[desc_table_index].inode_table * block_size + inode * superblock->inode_size) / block_size;
    void* buffer = ext2fs_load_block(block, superblock, block_id);

    // Copy
    memcpy(root, buffer + superblock->inode_size * inode_table_offset, sizeof(ext2fs_inode_t));

    // Deallocate the buffer
    free(buffer);
    return root;
}

// ext2_get_root_inode(generic_block_t*, ext2fs_superblock_t*, ext2fs_block_descriptor_t*) -> ext2fs_inode_t*
// Loads the root inode from an ext2 file system.
ext2fs_inode_t* ext2_get_root_inode(generic_block_t* block, ext2fs_superblock_t* superblock, ext2fs_block_descriptor_t* desc_table) {
    return ext2_load_inode(block, superblock, desc_table, 2);
}

struct s_dir_listing {
    unsigned int inode;
    unsigned short rec_len;
    unsigned char name_len;
    unsigned char file_type;
    char name[];
};

// ext2_fetch_from_directory(generic_block_t*, ext2fs_superblock_t*, ext2fs_block_descriptor_t*, ext2fs_inode_t*, char*) -> ext2fs_inode_t*
// Fetches an inode from a directory.
ext2fs_inode_t* ext2_fetch_from_directory(generic_block_t* block, ext2fs_superblock_t* superblock, ext2fs_block_descriptor_t* desc_table, ext2fs_inode_t* dir, char* file) {
    unsigned long long block_size = 1024 << superblock->log_block_size;
    for (int i = 0; i < INODE_DIRECT_COUNT; i++) {
        unsigned int block_id = dir->block[i];
        if (block_id == 0)
            continue;

        void* data = ext2fs_load_block(block, superblock, block_id);
        // uart_put_hexdump(data, 1024 << superblock->log_block_size);

        struct s_dir_listing* p = data;
        while (p < (struct s_dir_listing*) (data + block_size)) {
            // Compare string
            char name[p->name_len + 1];
            name[p->name_len] = 0;
            memcpy(name, p->name, p->name_len);
            if (!strcmp(name, file))
                return ext2_load_inode(block, superblock, desc_table, p->inode);

            // Get next entry
            unsigned char name_len = p->name_len;
            p = ((void*) (p + 1)) + name_len;

            // Padding
            p = (struct s_dir_listing*) (((unsigned long long) (((void*) p) + 3)) & ~3);
        }

        free(data);
    }
    // TODO: indirect blocks
    return 0;
}

ext2fs_inode_t* ext2_get_inode(generic_block_t* block, ext2fs_superblock_t* superblock, ext2fs_block_descriptor_t* desc_table, ext2fs_inode_t* root, char** path, unsigned long long path_node_count) {
    ext2fs_inode_t* node = root;
    for (unsigned long long i = 0; i < path_node_count; i++) {
        char* path_node = path[i];

    }

    return node;
}

