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
    int (*write_char)(generic_file_t*, int);
    struct s_dir_entry (*lookup)(generic_dir_t*, char*);
    struct s_dir_entry* (*list)(generic_dir_t*);
    unsigned long long (*size)(generic_file_t*);
    void (*seek)(generic_file_t*, unsigned long long);
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
    unsigned short permissions;

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
    unsigned long long buffer_indices[BUFFER_COUNT];
    unsigned long long buffer_block_indices[BUFFER_COUNT + 1];
    void* metadata_buffer;
    void* buffers[BUFFER_COUNT];
    unsigned char written_buffers[(BUFFER_COUNT + 1) / 8];
};

// generic_file_list(generic_dir_t*) -> struct s_dir_entry*
// Returns a list of directory entries.
static inline struct s_dir_entry* generic_dir_list(generic_dir_t* dir) {
    return (*dir)->fs.list(dir);
}

// generic_file_size(generic_file_t* file) -> unsigned long long
// Returns the size of the file.
static inline unsigned long long generic_file_size(generic_file_t* file) {
    return file->fs->size(file);
}

// generic_file_read_char(generic_file_t*) -> int
// Reads a character from a file, returning EOF if no more characters are available.
static inline int generic_file_read_char(generic_file_t* file) {
    if (file->type == GENERIC_FILE_TYPE_REGULAR && generic_file_size(file) > file->pos && file->fs->read_char)
        return file->fs->read_char(file);
    else
        return EOF;
}

// generic_file_write_char(generic_file_t*, int) -> int
// Writes a character to a file. Returns 0 on success.
static inline int generic_file_write_char(generic_file_t* file, int c) {
    if (file->type == GENERIC_FILE_TYPE_REGULAR && file->fs->write_char)
        return file->fs->write_char(file, c);
    return -1;
}

static inline void generic_file_seek(generic_file_t* file, unsigned long long pos) {
    unsigned long long size = generic_file_size(file);
    if (pos >= size) {
        pos = size;
        return;
    } else if (pos == file->pos)
        return;

    file->fs->seek(file, pos);
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

extern generic_dir_t* root;

#endif /* KERNEL_FS_GENERIC_H */

