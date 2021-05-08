#include "generic_file.h"
#include "../memory.h"
#include "../string.h"

#define INITIAL_SIZE 8
#define MOUNT_FUNC_COUNT 64

char (*mount_functions[MOUNT_FUNC_COUNT])(generic_block_t*, generic_filesystem_t*) = { 0 };

// register_fs_mounter(char (*)(generic_block_t*, generic_filesystem_t*)) -> void
// Register a file system mounter/driver.
void register_fs_mounter(char (*mounter)(generic_block_t*, generic_filesystem_t*)) {
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

    char fail = 1;
    for (unsigned int i = 0; fail && i < MOUNT_FUNC_COUNT; i++) {
        if (mount_functions[i](block, &fs) == 0) {
            fail = 0;
            break;
        }
    }

    if (!fail)
        (*dir)->fs = fs;
    return fail;
}

// init_generic_dir() -> generic_dir_t*
// Initialises a generic directory.
generic_dir_t* init_generic_dir() {
    generic_dir_t* dir = malloc(sizeof(generic_dir_t));
    *dir = malloc(sizeof(struct s_generic_dir) + sizeof(struct s_dir_entry) * INITIAL_SIZE);
    (*dir)->fs = (generic_filesystem_t) { 0 };
    (*dir)->value = (void*) 0;
    (*dir)->length = 0;
    (*dir)->size = INITIAL_SIZE;
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
}

// generic_dir_lookup_dir(generic_dir_t*, char*) -> struct s_dir_entry*
// Returns a pointer to the entry with the same name if found. Returns null if not found.
struct s_dir_entry* generic_dir_lookup_dir(generic_dir_t* dir, char* name) {
    struct s_dir_entry* entries = (*dir)->entries;
    unsigned long long length = (*dir)->length;
    for (unsigned long long i = 0; i < length; i++) {
        if (!strcmp(entries[i].name, name)) {
            return &entries[i];
        }
    }

    return (void*) 0;
}
