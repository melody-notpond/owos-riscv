#ifndef KERNEL_FS_GENERIC_H
#define KERNEL_FS_GENERIC_H

#include "../generic_block.h"

#define EOF (-1)

// BUFFER_COUNT mod 8 must be 7
#define BUFFER_COUNT 15

typedef enum {
    GENERIC_FILE_TYPE_DIR,
    GENERIC_FILE_TYPE_REGULAR,
} generic_file_type_t;

typedef struct s_generic_file generic_file_t;

typedef struct {
    void* mount;
    int (*read_char)(generic_file_t*);
} generic_filesystem_t;

typedef enum {
    DIR_ENTRY_TYPE_DIR,
    DIR_ENTRY_TYPE_MOUNT,
    DIR_ENTRY_TYPE_BLOCK,
} dir_entry_type_t;

struct s_dir_entry {
    char* name;

    dir_entry_type_t tag;
    union {
        struct s_generic_dir** dir;

        generic_block_t block;
    } value;
};

typedef struct s_generic_dir {
    generic_filesystem_t fs;
    generic_file_t* value;

    // List of mounted file systems and directories that contain mounted file systems
    unsigned long long length;
    unsigned long long size;
    struct s_dir_entry entries[];
} *generic_dir_t;

struct s_generic_file {
    generic_file_type_t type;
    generic_filesystem_t* fs;
    unsigned long long pos;
    unsigned char current_buffer;
    unsigned long long buffer_pos;
    unsigned long long buffer_block_indices[BUFFER_COUNT + 1];
    void* metadata_buffer;
    void* buffers[BUFFER_COUNT];
    unsigned char written_buffers[(BUFFER_COUNT + 1) / 8];
};

// generic_file_read_char(generic_file_t*) -> int
// Reads a character from a file, returning EOF if no more characters are available.
static inline int generic_file_read_char(generic_file_t* file) {
    if (file->type == GENERIC_FILE_TYPE_REGULAR)
        return file->fs->read_char(file);
    else
        return EOF;
}

// register_fs_mounter(char (*)(generic_block_t*, generic_filesystem_t*)) -> void
// Register a file system mounter/driver.
void register_fs_mounter(char (*mounter)(generic_block_t*, generic_filesystem_t*));

// mount_block_device(generic_dir_t*, generic_block_t*) -> void
// Mounts a block device. Returns 0 on success.
char mount_block_device(generic_dir_t* dir, generic_block_t* block);

// init_generic_dir() -> generic_dir_t*
// Initialises a generic directory.
generic_dir_t* init_generic_dir();

// generic_dir_append_entry(generic_dir_t*, struct s_dir_entry) -> void
// Appends an entry to a directory.
void generic_dir_append_entry(generic_dir_t* dir, struct s_dir_entry entry);

// generic_dir_lookup_dir(generic_dir_t*, char*) -> struct s_dir_entry*
// Returns a pointer to the entry with the same name if found. Returns null if not found.
struct s_dir_entry* generic_dir_lookup_dir(generic_dir_t* dir, char* name);

#endif /* KERNEL_FS_GENERIC_H */

