#ifndef KERNEL_FS_GENERIC_H
#define KERNEL_FS_GENERIC_H

#define EOF (-1)

// BUFFER_COUNT mod 8 must be 7
#define BUFFER_COUNT 15

typedef struct s_generic_file generic_file_t;

typedef struct {
    void* mount;
    int (*read_char)(generic_file_t*);
} generic_filesystem_t;

typedef enum {
    GENERIC_FILE_TYPE_DIR,
    GENERIC_FILE_TYPE_REGULAR,
} generic_file_type_t;

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

#endif /* KERNEL_FS_GENERIC_H */

