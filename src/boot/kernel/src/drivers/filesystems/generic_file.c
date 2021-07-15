#include "generic_file.h"
#include "../../lib/memory.h"
#include "../../lib/string.h"

#define INITIAL_SIZE 8
#define MOUNT_FUNC_COUNT 64

char (*mount_functions[MOUNT_FUNC_COUNT])(generic_block_t*, generic_file_t*) = { 0 };

generic_filesystem_t console_fs;

// register_fs_mounter(char (*)(generic_block_t*, generic_file_t*)) -> void
// Register a file system mounter/driver.
void register_fs_mounter(char (*mounter)(generic_block_t*, generic_file_t*)) {
    for (unsigned int i = 0; i < MOUNT_FUNC_COUNT; i++) {
        if (mount_functions[i] == 0) {
            mount_functions[i] = mounter;
            break;
        }
    }
}

// mount_block_device(generic_file_t*, generic_block_t*) -> void
// Mounts a block device. Returns 0 on success.
char mount_block_device(generic_file_t* dir, generic_block_t* block) {
    if (dir->type != GENERIC_FILE_TYPE_DIR || (*dir->dir)->mountpoint)
        return 1;

    char fail = 1;
    for (unsigned int i = 0; fail && i < MOUNT_FUNC_COUNT; i++) {
        if (mount_functions[i](block, dir) == 0) {
            fail = 0;
            break;
        }
    }

    if (!fail)
        (*dir->dir)->mountpoint = 1;
    return fail;
}

// close_generic_file(generic_file_t*) -> void
// Closes a generic file.
void close_generic_file(generic_file_t* file) {
    if (file == (void*) 0)
        return;

    // TODO: save changes

    switch (file->type) {
        case GENERIC_FILE_TYPE_DIR: {
            generic_dir_t d = *file->dir;
            for (unsigned long long i = 0; i < d->length; i++) {
                if (d->entries[i].name != (void*) 0)
                    free(d->entries[i].name);
                close_generic_file(d->entries[i].file);
            }

            if (d->buffer != (void*) 0) {
                free(d->buffer->metadata_buffer);
                for (int i = 0; i < BUFFER_COUNT; i++) {
                    free(d->buffer->buffers[i]);
                }
                free(d->buffer);
            }

            free(d);
            free(file->dir);
            break;
        }

        case GENERIC_FILE_TYPE_REGULAR:
            free(file->buffer->metadata_buffer);
            for (int i = 0; i < BUFFER_COUNT; i++) {
                free(file->buffer->buffers[i]);
            }
            free(file->buffer);
            break;

        case GENERIC_FILE_TYPE_BLOCK:
            free(file->block);
            break;

        case GENERIC_FILE_TYPE_SPECIAL:
            break;

        case GENERIC_FILE_TYPE_UNKNOWN:
            break;
    }

    // Remove entry from parent directory
    if (file->parent) {
        generic_dir_t parent = *file->parent->dir;
        for (unsigned long long i = 0; i < parent->length; i++) {
            if (parent->entries[i].file == file) {
                parent->entries[i].file = 0;
                free(parent->entries[i].name);
                parent->entries[i].name = (void*) 0;
                break;
            }
        }
    }

    if (file->fs->rc != -1 && (--file->fs->rc) == 0)
        free(file->fs);

    free(file);
}

// cleanup_directory(generic_file_t*) -> char
// Cleans up a directory by shifting its contents to remove unused space and deallocating unused folders.
char cleanup_directory(generic_file_t* dir) {
    if (dir->type != GENERIC_FILE_TYPE_DIR)
        return 0;

    generic_dir_t d = *dir->dir;
    unsigned long long diff = 0;
    for (unsigned long long i = 0; i < d->length; i++) {
        if (d->entries[i].file != ((void*) 0) && d->entries[i].file->type == GENERIC_FILE_TYPE_DIR && cleanup_directory(d->entries[i].file)) {
            d->entries[i].file = (void*) 0;
            free(d->entries[i].name);
        }

        if (d->entries[i].file == (void*) 0) {
            dir++;
        } else {
            d->entries[i - diff] = d->entries[i];
        }
    }

    d->length -= diff;
    if (d->length == 0 && !d->mountpoint) {
        close_generic_file(dir);
        return 1;
    }

    return 0;
}

// unmount_generic_dir(generic_file_t* dir) -> char
// Unmounts a generic directory. Returns 0 on success.
char unmount_generic_dir(generic_file_t* dir) {
    if (dir->type != GENERIC_FILE_TYPE_DIR)
        return 1;

    generic_dir_t d = *dir->dir;
    if (dir->fs->mount == 0 || !d->mountpoint)
        return 1;

    // TODO: check if the device is busy

    char status = dir->fs->unmount(dir);
    if (!status) {
        close_generic_file(dir);
        return 0;
    }
    return status;
}

// init_generic_dir() -> generic_dir_t*
// Initialises a generic directory.
generic_dir_t* init_generic_dir() {
    generic_dir_t* dir = malloc(sizeof(generic_dir_t));
    *dir = malloc(sizeof(struct s_generic_dir) + sizeof(struct s_dir_entry) * INITIAL_SIZE);
    **dir = (struct s_generic_dir) {
        .mountpoint = 0,
        .length = 0,
        .size = INITIAL_SIZE
    };
    return dir;
}

// generic_dir_append_entry(generic_file_t*, struct s_dir_entry) -> void
// Appends an entry to a directory.
void generic_dir_append_entry(generic_file_t* dir, struct s_dir_entry entry) {
    if (dir->type != GENERIC_FILE_TYPE_DIR)
        return;

    generic_dir_t d = *dir->dir;
    if (d->length >= d->size) {
        // Resize list
        unsigned long long size = d->size << 1;
        *dir->dir = realloc(*dir->dir, sizeof(struct s_generic_dir) + sizeof(struct s_dir_entry) * size);
        d = *dir->dir;
        d->size = size;
    }

    // Append entry
    d->entries[d->length++] = entry;
    entry.file->parent = dir;
}

// generic_dir_lookup_dir(generic_file_t*, generic_dir_t*, char*) -> struct s_dir_entry*
// Returns an entry with the same name if found. Returns a zeroed out structure if not found.
struct s_dir_entry generic_dir_lookup_dir(generic_file_t* file, char* name) {
    // Check for . and ..
    if (!strcmp(name, "."))
        return (struct s_dir_entry) {
            .name = ".",
            .file = file
        };
    else if (!strcmp(name, ".."))
        return (struct s_dir_entry) {
            .name = "..",
            .file = file->parent
        };

    // Check cached entries first
    generic_dir_t dir = *file->dir;
    struct s_dir_entry* entries = dir->entries;
    unsigned long long length = dir->length;
    for (unsigned long long i = 0; i < length; i++) {
        if (!strcmp(entries[i].name, name))
            return entries[i];
    }

    // Lookup via file system driver
    struct s_dir_entry entry = file->fs->lookup(file, name);
    if (entry.file != (void*) 0) {
        entry.file->fs = file->fs;
        file->fs->rc++;
        if (entry.file->type == GENERIC_FILE_TYPE_DIR)
            generic_dir_append_entry(file, entry);
    }
    return entry;
}

// generic_dir_lookup(generic_file_t*, char*) -> struct s_dir_entry
// Returns an entry with the same name if found. Returns a zeroed out structure if not found.
struct s_dir_entry generic_dir_lookup(generic_file_t* dir, char* path) {
    struct s_dir_entry entry = { 0 };
    if (path[0] == '/') {
        dir = root;
        entry = (struct s_dir_entry) {
            .name = "/",
            .file = dir,
        };
    }

    char* path_buffer = malloc(strlen(path) + 1);
    path_buffer[0] = 0;
    unsigned long long i = 0;
    for (char* p = path; *p; p++) {
        if (*p == '/') {
            path_buffer[i] = 0;
            i = 0;

            if (path_buffer[0] == 0)
                continue;

            entry = generic_dir_lookup_dir(dir, path_buffer);
            if (entry.file == (void*) 0 || entry.file->type != GENERIC_FILE_TYPE_DIR) {
                free(path_buffer);
                return (struct s_dir_entry) { 0 };
            }

            dir = entry.file;
        } else {
            path_buffer[i++] = *p;
        }
    }

    path_buffer[i] = 0;
    if (path_buffer[0] != 0) {
        entry = generic_dir_lookup_dir(dir, path_buffer);
    }

    free(path_buffer);
    return entry;
}

// generic_file_read(generic_file_t*, void*, unsigned long long) -> unsigned long long
// Reads binary data from a file. Returns the number of bytes read.
unsigned long long generic_file_read(generic_file_t* file, void* buffer, unsigned long long size) {
    char* p = buffer;
    if (p == (void*) 0)
        return 0;

    for (unsigned long long i = 0; i < size; i++) {
        int c;
        if ((c = generic_file_read_char(file)) == EOF)
            return i;
        p[i] = c;
    }

    return size;
}

// generic_file_write(generic_file_t*, void*, unsigned long long) -> unsigned long long
// Writes binary data to a file. Returns the number of bytes written.
unsigned long long generic_file_write(generic_file_t* file, void* buffer, unsigned long long size) {
    char* p = buffer;
    if (p == (void*) 0)
        return 0;

    for (unsigned long long i = 0; i < size; i++) {
        if (generic_file_write_char(file, p[i]))
            return i;
    }

    return size;
}

// clean_generic_entry_listing(struct s_dir_entry*) -> void
// Cleans a list of entries returned by generic_dir_list().
void clean_generic_entry_listing(struct s_dir_entry* entries) {
    struct s_dir_entry* e = entries;
    while (e->file != (void*) 0) {
        free(e->name);
        e++;
    }

    free(entries);
}

// copy_generic_file(generic_file_t*, generic_file_t*) -> void
// Copies a generic file.
void copy_generic_file(generic_file_t* dest, generic_file_t* src) {
    dest->parent = src->parent;
    dest->fs = src->fs;
    dest->type = src->type;

    // TODO
}
