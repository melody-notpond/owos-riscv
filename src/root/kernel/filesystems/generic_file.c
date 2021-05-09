#include "generic_file.h"
#include "../memory.h"
#include "../string.h"

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
        (*dir)->fs = fs;
        (*dir)->value = malloc(sizeof(generic_file_t));
        *(*dir)->value = file;
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
    free(file);
}

// unmount_generic_dir(generic_dir_t* dir) -> char
// Unmounts a generic directory. Returns 0 on success.
char unmount_generic_dir(generic_dir_t* dir) {
    generic_dir_t d = *dir;
    if (d->fs.mount == 0)
        return 1;

    // TODO: check if the device is busy

    char status = d->fs.unmount(&d->fs, d->value);
    if (!status) {
        close_generic_file(d->value);
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
        .fs = (generic_filesystem_t) { 0 },
        .value = (void*) 0,
        .parent = dir,
        .length = 0,
        .size = INITIAL_SIZE,
    };
    return dir;
}

// generic_dir_append_entry(generic_dir_t*, struct s_dir_entry) -> void
// Appends an entry to a directory.
void generic_dir_append_entry(generic_dir_t* dir, struct s_dir_entry entry) {
    generic_dir_t d = *dir;
    if (d->length >= d->size) {
        unsigned long long size = d->size << 1;
        *dir = realloc(*dir, sizeof(struct s_generic_dir) + sizeof(struct s_dir_entry) * size);
        d = *dir;
        d->size = size;
    }

    d->entries[d->length++] = entry;
    if (entry.tag == DIR_ENTRY_TYPE_DIR) {
        (*entry.value.dir)->parent = dir;
    }
}

// generic_dir_lookup_dir(generic_dir_t*, char*) -> struct s_dir_entry*
// Returns a pointer to the entry with the same name if found. Returns null if not found.
struct s_dir_entry generic_dir_lookup_dir(generic_dir_t* dir, char* name) {
    // Check cached entries first
    struct s_dir_entry* entries = (*dir)->entries;
    unsigned long long length = (*dir)->length;
    for (unsigned long long i = 0; i < length; i++) {
        if (!strcmp(entries[i].name, name)) {
            return entries[i];
        }
    }

    // Lookup via file system driver
    return (*dir)->fs.lookup(dir, name);
}
