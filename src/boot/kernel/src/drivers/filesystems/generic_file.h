#ifndef KERNEL_FS_GENERIC_H
#define KERNEL_FS_GENERIC_H

#include "../generic_block.h"

#define EOF (-1)

// BUFFER_COUNT mod 8 must be 7
#define BUFFER_COUNT 15

typedef enum {
    GENERIC_FILE_TYPE_UNKNOWN,
    GENERIC_FILE_TYPE_DIR,
    GENERIC_FILE_TYPE_REGULAR,
} generic_file_type_t;

typedef struct s_generic_file generic_file_t;
typedef struct s_generic_dir *generic_dir_t;
struct s_dir_entry;

typedef struct s_generic_filesystem {
    void* mount;
    char (*unmount)(struct s_generic_filesystem*, generic_file_t*);
    int (*read_char)(generic_file_t*);
    struct s_dir_entry (*lookup)(generic_dir_t*, char*);
    unsigned long long (*size)(generic_file_t* file);
} generic_filesystem_t;

typedef enum {
    DIR_ENTRY_TYPE_UNUSED = 0,
    DIR_ENTRY_TYPE_UNKNOWN,
    DIR_ENTRY_TYPE_DIR,
    DIR_ENTRY_TYPE_BLOCK,
    DIR_ENTRY_TYPE_REGULAR,
} dir_entry_type_t;

struct s_dir_entry {
    char* name;

    dir_entry_type_t tag;
    union {
        struct s_generic_dir** dir;
        generic_block_t* block;
        generic_file_t* file;
    } value;
};

struct s_generic_dir {
    char mountpoint;
    generic_filesystem_t fs;
    generic_file_t* value;
    generic_dir_t* parent;

    // List of mounted file systems and directories that contain mounted file systems
    unsigned long long length;
    unsigned long long size;
    struct s_dir_entry entries[];
};

struct s_generic_file {
    generic_file_type_t type;
    generic_filesystem_t* fs;
    generic_dir_t* parent;
    unsigned long long pos;
    unsigned char current_buffer;
    unsigned long long buffer_pos;
    unsigned long long buffer_block_indices[BUFFER_COUNT + 1];
    void* metadata_buffer;
    void* buffers[BUFFER_COUNT];
    unsigned char written_buffers[(BUFFER_COUNT + 1) / 8];
};

// generic_file_size(generic_file_t* file) -> unsigned long long
// Returns the size of the file.
static inline unsigned long long generic_file_size(generic_file_t* file) {
    return file->fs->size(file);
}

// generic_file_read_char(generic_file_t*) -> int
// Reads a character from a file, returning EOF if no more characters are available.
static inline int generic_file_read_char(generic_file_t* file) {
    if (file->type == GENERIC_FILE_TYPE_REGULAR && generic_file_size(file) > file->pos)
        return file->fs->read_char(file);
    else
        return EOF;
}

// register_fs_mounter(char (*)(generic_block_t*, generic_filesystem_t*, generic_file_t*)) -> void
// Register a file system mounter/driver.
void register_fs_mounter(char (*mounter)(generic_block_t*, generic_filesystem_t*, generic_file_t*));

// mount_block_device(generic_dir_t*, generic_block_t*) -> void
// Mounts a block device. Returns 0 on success.
char mount_block_device(generic_dir_t* dir, generic_block_t* block);

// close_generic_file(generic_file_t*) -> void
// Closes a generic file.
void close_generic_file(generic_file_t* file);

// cleanup_directory(generic_dir_t*) -> char
// Cleans up a directory by shifting its contents to remove unused space and deallocating unused folders.
char cleanup_directory(generic_dir_t* dir);

// unmount_generic_dir(generic_dir_t* dir) -> char
// Unmounts a generic directory. Returns 0 on success.
char unmount_generic_dir(generic_dir_t* dir);

// init_generic_dir() -> generic_dir_t*
// Initialises a generic directory.
generic_dir_t* init_generic_dir();

// generic_dir_append_entry(generic_dir_t*, struct s_dir_entry) -> void
// Appends an entry to a directory.
void generic_dir_append_entry(generic_dir_t* dir, struct s_dir_entry entry);

// generic_dir_lookup(generic_dir_t*, char*) -> struct s_dir_entry
// Returns an entry with the same name if found. Returns a zeroed out structure if not found.
struct s_dir_entry generic_dir_lookup(generic_dir_t* dir, char* path);

#endif /* KERNEL_FS_GENERIC_H */

