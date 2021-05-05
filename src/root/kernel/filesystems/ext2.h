#ifndef KERNEL_FS_EXT2_H
#define KERNEL_FS_EXT2_H

#include "../generic_block.h"

typedef struct __attribute__((__packed__, aligned(1))) {
    unsigned int inodes_count;
    unsigned int blocks_count;
    unsigned int r_blocks_count;
    unsigned int free_blocks_count;

    unsigned int free_inodes_count;
    unsigned int first_data_block;
    unsigned int log_block_size;
    unsigned int log_frag_size;

    unsigned int blocks_per_group;
    unsigned int frags_per_group;
    unsigned int inodes_per_group;
    unsigned int mtime;

    unsigned int wtime;
    unsigned short mnt_count;
    unsigned short max_mnt_count;
    unsigned short magic;
    unsigned short state;
    unsigned short errors;
    unsigned short minor_rev_level;

    unsigned int last_check;
    unsigned int check_interval;
    unsigned int creator_os;
    unsigned int rev_level;

    unsigned short def_resuid;
    unsigned short def_resgid;
    unsigned int first_ino;
    unsigned short inode_size;
    unsigned short block_group_nr;
    unsigned int feature_compat;

    unsigned int feature_incompat;
    unsigned int feature_ro_compat;

    unsigned char uuid[16];

    unsigned char volume_name[16];

    unsigned char last_mounted[64];

    unsigned int algo_bitmap;
    unsigned char prealloc_blocks;
    unsigned char prealloc_dir_blocks;

    unsigned char rsv1[2];

    unsigned char journal_uuid[16];

    unsigned int journal_inum;
    unsigned int journal_dev;
    unsigned int last_orphan;

    unsigned int hash_seed[4];
    unsigned char def_hash_version;
    unsigned char rsv2[3];

    unsigned int default_mount_options;
    unsigned int first_meta_bg;

    unsigned char rsv3[760];
} ext2fs_superblock_t;

typedef struct __attribute__((__packed__, aligned(2))) {
    unsigned int block_bitmap;
    unsigned int inode_bitmap;
    unsigned int inode_table;
    unsigned short free_blocks_count;
    unsigned short free_inodes_count;

    unsigned short used_dirs_count;
    unsigned char rsv1[14];
} ext2fs_block_descriptor_t;

typedef struct __attribute__((__packed__, aligned(2))) {
    unsigned short mode;
    unsigned short uid;
    unsigned int size;
    unsigned int atime;
    unsigned int ctime;

    unsigned int mtime;
    unsigned int dtime;
    unsigned short gid;
    unsigned short links_count;
    unsigned int blocks;

    unsigned int flags;
    unsigned int osd1;

    unsigned int block[15];

    unsigned int generation;
    unsigned int file_acl;
    unsigned int dir_acl;
    unsigned int faddr;

    unsigned char osd2[12];
} ext2fs_inode_t;

// ext2_load_superblock(generic_block_t*) -> ext2fs_superblock_t*
// Loads a superblock from a block device.
ext2fs_superblock_t* ext2_load_superblock(generic_block_t* block);

// Temporarily public
void* ext2fs_load_block(generic_block_t* block, ext2fs_superblock_t* superblock, unsigned int block_id);

// ext2_load_block_descriptor_table(generic_block_t*, ext2fs_superblock_t*) -> ext2fs_block_descriptor_t*
// Loads a descriptor table from an ext2 file system.
ext2fs_block_descriptor_t* ext2_load_block_descriptor_table(generic_block_t* block, ext2fs_superblock_t* superblock);

// ext2_get_root_inode(generic_block_t*, ext2fs_superblock_t*, ext2fs_block_descriptor_t*) -> ext2fs_inode_t*
// Loads the root inode from an ext2 file system.
ext2fs_inode_t* ext2_get_root_inode(generic_block_t* block, ext2fs_superblock_t* superblock, ext2fs_block_descriptor_t* desc_table);

// ext2_fetch_from_directory(generic_block_t*, ext2fs_superblock_t*, ext2fs_block_descriptor_t*, ext2fs_inode_t*, char*) -> ext2fs_inode_t*
// Fetches an inode from a directory.
ext2fs_inode_t* ext2_fetch_from_directory(generic_block_t* block, ext2fs_superblock_t* superblock, ext2fs_block_descriptor_t* desc_table, ext2fs_inode_t* dir, char* file);

// ext2_get_inode(generic_block_t*, ext2fs_superblock_t*, ext2fs_block_descriptor_t*, ext2fs_inode_t*, char**, unsigned long long) -> ext2fs_inode_t*
// Gets an inode by walking the path from the root inode.
ext2fs_inode_t* ext2_get_inode(generic_block_t* block, ext2fs_superblock_t* superblock, ext2fs_block_descriptor_t* desc_table, ext2fs_inode_t* root, char** path, unsigned long long path_node_count);

#endif /* KERNEL_FS_EXT2_H */

