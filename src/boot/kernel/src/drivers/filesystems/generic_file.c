#include "generic_file.h"
#include "../../lib/memory.h"
#include "../../lib/string.h"

#define INITIAL_SIZE 8
#define MOUNT_FUNC_COUNT 64

char (*mount_functions[MOUNT_FUNC_COUNT])(generic_block_t*, generic_filesystem_t*, generic_file_t*) = { 0 };

// register_fs_mounter(char (*)(generic_block_t*, generic_filesystem_t*, generic_file_t*)) -> void
// Register a file system mounter/driver.
void register_fs_mounter(char (*mounter)(generic_block_t*, generic_filesystem_t*, generic_file_t*)) {
    for (unsigned int i = 0; i < MOUNT_FUNC_COUNT; i++) {
        if (mount_functions[i] == 0) {
            mount_functions[i] = mounter;
            break;
        }
    }
}

// mount_block_device(generic_dir_t*, generic_block_t*) -> void
// Mounts a block device. Returns 0 on success.
char mount_block_device(generic_dir_t* dir, generic_block_t* block) {
    generic_filesystem_t fs;
    generic_file_t file;

    char fail = 1;
    for (unsigned int i = 0; fail && i < MOUNT_FUNC_COUNT; i++) {
        if (mount_functions[i](block, &fs, &file) == 0) {
            fail = 0;
            break;
        }
    }

    if (!fail) {
        file.parent = (void*) 0;
        (*dir)->fs = fs;
        (*dir)->value = malloc(sizeof(generic_file_t));
        *(*dir)->value = file;
        (*dir)->mountpoint = 1;
    }
    return fail;
}

// close_generic_file(generic_file_t*) -> void
// Closes a generic file.
void close_generic_file(generic_file_t* file) {
    if (file == (void*) 0)
        return;

    // TODO: save changes

    free(file->metadata_buffer);
    for (unsigned int i = 0; i < BUFFER_COUNT; i++) {
        free(file->buffers[i]);
    }

    // Remove entry from parent directory
    if (file->parent) {
        generic_dir_t parent = *file->parent;
        for (unsigned long long i = 0; i < parent->length; i++) {
            if (parent->entries[i].value.file == file) {
                parent->entries[i].value.file = 0;
                parent->entries[i].tag = DIR_ENTRY_TYPE_UNUSED;
                free(parent->entries[i].name);
                break;
            }
        }
    }

    free(file);
}

// cleanup_directory(generic_dir_t*) -> char
// Cleans up a directory by shifting its contents to remove unused space and deallocating unused folders.
char cleanup_directory(generic_dir_t* dir) {
    generic_dir_t d = *dir;
    unsigned long long diff = 0;
    for (unsigned long long i = 0; i < d->length; i++) {
        if (d->entries[i].tag == DIR_ENTRY_TYPE_DIR && cleanup_directory(d->entries[i].value.dir)) {
            d->entries[i].tag = DIR_ENTRY_TYPE_UNUSED;
            free(d->entries[i].name);
        }

        if (d->entries[i].tag == DIR_ENTRY_TYPE_UNUSED) {
            dir++;
        } else {
            d->entries[i - diff] = d->entries[i];
        }
    }

    d->length -= diff;
    if (d->length == 0 && !d->mountpoint) {
        close_generic_file(d->value);
        return 1;
    }

    return 0;
}

// recursive_remove_directories(generic_dir_t*) -> void
// Recursively removes directories.
void recursive_remove_directories(generic_dir_t* dir) {
    generic_dir_t d = *dir;

    for (unsigned long long i = 0; i < d->length; i++) {
        if (d->entries[i].tag != DIR_ENTRY_TYPE_UNUSED)
            free(d->entries[i].name);

        if (d->entries[i].tag == DIR_ENTRY_TYPE_DIR) {
            recursive_remove_directories(d->entries[i].value.dir);
        } else if (d->entries[i].tag == DIR_ENTRY_TYPE_REGULAR) {
            close_generic_file(d->entries[i].value.file);
        } else if (d->entries[i].tag == DIR_ENTRY_TYPE_BLOCK) {
            free(d->entries[i].value.block);
        }
    }

    close_generic_file(d->value);
    free(*dir);
    free(dir);
}

// unmount_generic_dir(generic_dir_t* dir) -> char
// Unmounts a generic directory. Returns 0 on success.
char unmount_generic_dir(generic_dir_t* dir) {
    generic_dir_t d = *dir;
    if (d->fs.mount == 0 || !d->mountpoint)
        return 1;

    // TODO: check if the device is busy

    char status = d->fs.unmount(&d->fs, d->value);
    if (!status) {
        recursive_remove_directories(dir);
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
        .fs = (generic_filesystem_t) { 0 },
        .value = (void*) 0,
        .parent = dir,
        .length = 0,
        .size = INITIAL_SIZE
    };
    return dir;
}

// generic_dir_append_entry(generic_dir_t*, struct s_dir_entry) -> void
// Appends an entry to a directory.
void generic_dir_append_entry(generic_dir_t* dir, struct s_dir_entry entry) {
    generic_dir_t d = *dir;
    if (d->length >= d->size) {
        // Resize list
        unsigned long long size = d->size << 1;
        *dir = realloc(*dir, sizeof(struct s_generic_dir) + sizeof(struct s_dir_entry) * size);
        d = *dir;
        d->size = size;
    }

    // Append entry
    d->entries[d->length++] = entry;
    if (entry.tag == DIR_ENTRY_TYPE_DIR) {
        // Directories get their parents marked
        (*entry.value.dir)->parent = dir;
    }
}

// generic_dir_lookup_dir(generic_dir_t*, char*) -> struct s_dir_entry*
// Returns an entry with the same name if found. Returns a zeroed out structure if not found.
struct s_dir_entry generic_dir_lookup_dir(generic_dir_t* dir, char* name) {
    // Check for . and ..
    if (!strcmp(name, "."))
        return (struct s_dir_entry) {
            .tag = DIR_ENTRY_TYPE_DIR,
            .name = ".",
            .value = {
                .dir = dir
            }
        };
    else if (!strcmp(name, ".."))
        return (struct s_dir_entry) {
            .tag = DIR_ENTRY_TYPE_DIR,
            .name = "..",
            .value = {
                .dir = (*dir)->parent
            }
        };

    // Check cached entries first
    struct s_dir_entry* entries = (*dir)->entries;
    unsigned long long length = (*dir)->length;
    for (unsigned long long i = 0; i < length; i++) {
        if (!strcmp(entries[i].name, name)) {
            return entries[i];
        }
    }

    // Lookup via file system driver
    struct s_dir_entry entry = (*dir)->fs.lookup(dir, name);
    if (entry.tag == DIR_ENTRY_TYPE_REGULAR)
        entry.value.file->fs = &(*dir)->fs;
    else if (entry.tag == DIR_ENTRY_TYPE_DIR) {
        (*entry.value.dir)->fs = (*dir)->fs;
        generic_dir_append_entry(dir, entry);
    }
    return entry;
}

// generic_dir_lookup(generic_dir_t*, char*) -> struct s_dir_entry
// Returns an entry with the same name if found. Returns a zeroed out structure if not found.
struct s_dir_entry generic_dir_lookup(generic_dir_t* dir, char* path) {
    char* path_buffer = malloc(strlen(path) + 1);
    path_buffer[0] = 0;
    unsigned long long i = 0;
    struct s_dir_entry entry = { 0 };
    for (char* p = path; *p; p++) {
        if (*p == '/') {
            path_buffer[i] = 0;
            i = 0;

            if (path_buffer[0] == 0)
                continue;

            entry = generic_dir_lookup_dir(dir, path_buffer);
            if (entry.tag != DIR_ENTRY_TYPE_DIR) {
                free(path_buffer);
                return (struct s_dir_entry) { 0 };
            }

            dir = entry.value.dir;
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

// generic_file_read(generic_file_t*, void*, unsigned long long) -> void
// Reads binary data from a file.
void generic_file_read(generic_file_t* file, void* buffer, unsigned long long size) {
    char* p = buffer;
    for (unsigned long long i = 0; i < size; i++) {
        int c;
        if ((c = generic_file_read_char(file)) == EOF)
            break;
        if (p != (void*) 0)
            p[i] = c;
    }
}
