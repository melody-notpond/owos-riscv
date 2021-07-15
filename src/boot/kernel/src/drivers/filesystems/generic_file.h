#ifndef KERNEL_FS_GENERIC_H
#define KERNEL_FS_GENERIC_H

#include "../generic_block.h"

#define EOF (-1)

// BUFFER_COUNT mod 8 must be 7
#define BUFFER_COUNT 15

typedef enum {
    GENERIC_FILE_TYPE_UNKNOWN = 0,
    GENERIC_FILE_TYPE_DIR,
    GENERIC_FILE_TYPE_REGULAR,
    GENERIC_FILE_TYPE_BLOCK,
    GENERIC_FILE_TYPE_SPECIAL,
} generic_file_type_t;

typedef struct s_generic_file generic_file_t;
typedef struct s_generic_dir *generic_dir_t;
struct s_dir_entry;

typedef struct s_generic_filesystem {
    unsigned long long rc;
    void* mount;
    char (*unmount)(generic_file_t*);
    int (*read_char)(generic_file_t*);
    int (*write_char)(generic_file_t*, int);
    struct s_dir_entry (*lookup)(generic_file_t*, char*);
    struct s_dir_entry* (*list)(generic_file_t*);
    unsigned long long (*size)(generic_file_t*);
    void (*seek)(generic_file_t*, unsigned long long);
} generic_filesystem_t;

struct s_dir_entry {
    char* name;
    generic_file_t* file;
};

typedef struct {
    unsigned long long pos;
    unsigned char current_buffer;
    unsigned long long buffer_pos;
    unsigned long long buffer_indices[BUFFER_COUNT];
    unsigned long long buffer_block_indices[BUFFER_COUNT + 1];
    void* metadata_buffer;
    void* buffers[BUFFER_COUNT];
    unsigned char written_buffers[(BUFFER_COUNT + 1) / 8];
} generic_file_buffer_t;

struct s_generic_dir {
    char mountpoint;
    generic_file_buffer_t* buffer;

    // List of mounted file systems and directories that contain mounted file systems
    unsigned long long length;
    unsigned long long size;
    struct s_dir_entry entries[];
};

struct s_generic_file {
    // Metadata
    unsigned short permissions;
    generic_filesystem_t* fs;
    generic_file_t* parent;

    // Data
    generic_file_type_t type;
    union {
        generic_file_buffer_t* buffer;
        generic_dir_t* dir;
        generic_block_t* block;
    };
};

// generic_file_list(generic_file_t*) -> struct s_dir_entry*
// Returns a list of directory entries.
static inline struct s_dir_entry* generic_dir_list(generic_file_t* dir) {
    if (dir->type == GENERIC_FILE_TYPE_DIR)
        return dir->fs->list(dir);
    return (struct s_dir_entry*) 0;
}

// generic_file_size(generic_file_t* file) -> unsigned long long
// Returns the size of the file.
static inline unsigned long long generic_file_size(generic_file_t* file) {
    return file->fs->size(file);
}

// generic_file_read_char(generic_file_t*) -> int
// Reads a character from a file, returning EOF if no more characters are available.
static inline int generic_file_read_char(generic_file_t* file) {
    if (((file->type == GENERIC_FILE_TYPE_REGULAR && generic_file_size(file) > file->buffer->pos) || file->type == GENERIC_FILE_TYPE_SPECIAL) && file->fs->read_char)
        return file->fs->read_char(file);
    else
        return EOF;
}

// generic_file_write_char(generic_file_t*, int) -> int
// Writes a character to a file. Returns 0 on success.
static inline int generic_file_write_char(generic_file_t* file, int c) {
    if ((file->type == GENERIC_FILE_TYPE_REGULAR || file->type == GENERIC_FILE_TYPE_SPECIAL) && file->fs->write_char)
        return file->fs->write_char(file, c);
    return -1;
}

// generic_file_seek(generic_file_t*, unsigned long long) -> void
// Seeks to a position in the file.
static inline void generic_file_seek(generic_file_t* file, unsigned long long pos) {
    if (file->type != GENERIC_FILE_TYPE_REGULAR)
        return;

    unsigned long long size = generic_file_size(file);
    if (pos >= size) {
        pos = size;
        return;
    } else if (pos == file->buffer->pos)
        return;

    file->fs->seek(file, pos);
}

// register_fs_mounter(char (*)(generic_block_t*, generic_file_t*)) -> void
// Register a file system mounter/driver.
void register_fs_mounter(char (*mounter)(generic_block_t*, generic_file_t*));

// mount_block_device(generic_file_t*, generic_block_t*) -> void
// Mounts a block device. Returns 0 on success.
char mount_block_device(generic_file_t* dir, generic_block_t* block);

// close_generic_file(generic_file_t*) -> void
// Closes a generic file.
void close_generic_file(generic_file_t* file);

// cleanup_directory(generic_file_t*) -> char
// Cleans up a directory by shifting its contents to remove unused space and deallocating unused folders.
char cleanup_directory(generic_file_t* dir);

// unmount_generic_dir(generic_file_t*) -> char
// Unmounts a generic directory. Returns 0 on success.
char unmount_generic_dir(generic_file_t* dir);

// init_generic_dir() -> generic_dir_t*
// Initialises a generic directory.
generic_dir_t* init_generic_dir();

// generic_dir_append_entry(generic_file_t*, struct s_dir_entry) -> void
// Appends an entry to a directory.
void generic_dir_append_entry(generic_file_t* dir, struct s_dir_entry entry);

// generic_dir_lookup(generic_file_t*, char*) -> struct s_dir_entry
// Returns an entry with the same name if found. Returns a zeroed out structure if not found.
struct s_dir_entry generic_dir_lookup(generic_file_t* dir, char* path);

// generic_file_read(generic_file_t*, void*, unsigned long long) -> unsigned long long
// Reads binary data from a file. Returns the number of bytes read.
unsigned long long generic_file_read(generic_file_t* file, void* buffer, unsigned long long size);

// generic_file_write(generic_file_t*, void*, unsigned long long) -> unsigned long long
// Writes binary data to a file. Returns the number of bytes written.
unsigned long long generic_file_write(generic_file_t* file, void* buffer, unsigned long long size);

// clean_generic_entry_listing(struct s_dir_entry*) -> void
// Cleans a list of entries returned by generic_dir_list().
void clean_generic_entry_listing(struct s_dir_entry* entries);

// copy_generic_file(generic_file_t*, generic_file_t*) -> void
// Copies a generic file.
void copy_generic_file(generic_file_t* dest, generic_file_t* src);

extern generic_file_t* root;

#endif /* KERNEL_FS_GENERIC_H */

