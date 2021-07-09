#include "opensbi.h"
#include "drivers/filesystems/generic_file.h"
#include "drivers/console/console.h"
#include "kshell.h"
#include "lib/memory.h"
#include "lib/string.h"

#define COMMAND_SIZE 16

// kshell_main() -> void
// Does the shell.
void kshell_main() {
    generic_dir_t* current = root;
    console_puts("Welcome to the owOS kernel shell!\nType `help` for a list of commands.\n");

    while (1) {
        char* s = console_readline("> ");

        char command[COMMAND_SIZE] = { 0 };
        int i;
        for (i = 0; i < COMMAND_SIZE && s[i]; i++) {
            if (s[i] == ' ')
                break;
            else
                command[i] = s[i];
        }

        if (command[COMMAND_SIZE - 1] != 0) {
            console_printf("Command %s is dummy thicc.\n", s);
            free(s);
            continue;
        }

        if (command[0] == 0) {
        } else if (!strcmp(command, "ls")) {
            char* filename = s + i + 1;
            struct s_dir_entry entry = generic_dir_lookup(current, filename);

            switch (entry.tag) {
                case DIR_ENTRY_TYPE_DIR: {
                    struct s_dir_entry* entries = generic_dir_list(entry.value.dir);
                    struct s_dir_entry* e = entries;
                    console_puts("Entries of directory:\n");
                    while (e->tag != DIR_ENTRY_TYPE_UNUSED) {
                        console_printf("%s\n", e->name);
                        e++;
                    }
                    clean_generic_entry_listing(entries);
                    break;
                }

                case DIR_ENTRY_TYPE_REGULAR:
                    console_printf("%s is a regular file.\n", filename);
                    break;

                case DIR_ENTRY_TYPE_BLOCK:
                    console_printf("%s is a block device.\n", filename);
                    break;

                case DIR_ENTRY_TYPE_UNKNOWN:
                    console_printf("Unknown file type %s.\n", filename);
                    break;

                case DIR_ENTRY_TYPE_UNUSED:
                    console_printf("File %s not found.\n", filename);
                    break;
            }
        } else if (!strcmp(command, "cd")) {
            char* filename = s + i + 1;
            struct s_dir_entry entry = generic_dir_lookup(current, filename);

            switch (entry.tag) {
                case DIR_ENTRY_TYPE_DIR:
                    current = entry.value.dir;
                    break;

                case DIR_ENTRY_TYPE_REGULAR:
                    console_printf("%s is a regular file.\n", filename);
                    break;

                case DIR_ENTRY_TYPE_BLOCK:
                    console_printf("%s is a block device.\n", filename);
                    break;

                case DIR_ENTRY_TYPE_UNKNOWN:
                    console_printf("Unknown file type %s.\n", filename);
                    break;

                case DIR_ENTRY_TYPE_UNUSED:
                    console_printf("File %s not found.\n", filename);
                    break;
            }
        } else if (!strcmp(command, "hexdump")) {
            char* filename = s + i + 1;
            struct s_dir_entry entry = generic_dir_lookup(current, filename);

            switch (entry.tag) {
                case DIR_ENTRY_TYPE_DIR: {
                    console_printf("%s is a directory.\n", filename);
                    break;
                }

                case DIR_ENTRY_TYPE_REGULAR: {
                    generic_file_t* file = entry.value.file;
                    unsigned long long filesize = generic_file_size(file);
                    void* buffer = malloc(filesize);
                    generic_file_read(file, buffer, filesize);
                    console_put_hexdump(buffer, filesize);
                    free(buffer);
                    close_generic_file(file);
                    break;
                }

                case DIR_ENTRY_TYPE_BLOCK:
                    console_printf("%s is a block device.\n", filename);
                    break;

                case DIR_ENTRY_TYPE_UNKNOWN:
                    console_printf("Unknown file type %s.\n", filename);
                    break;

                case DIR_ENTRY_TYPE_UNUSED:
                    console_printf("File %s not found.\n", filename);
                    break;
            }
        } else if (!strcmp(command, "cat")) {
            char* filename = s + i + 1;
            struct s_dir_entry entry = generic_dir_lookup(current, filename);

            switch (entry.tag) {
                case DIR_ENTRY_TYPE_REGULAR: {
                    generic_file_t* file = entry.value.file;
                    int c;
                    while ((c = generic_file_read_char(file)) != EOF) {
                        sbi_console_putchar(c);
                    }
                    sbi_console_putchar('\n');
                    close_generic_file(file);
                    break;
                }

                case DIR_ENTRY_TYPE_DIR:
                    console_printf("%s is a directory.\n", filename);
                    break;

                case DIR_ENTRY_TYPE_BLOCK:
                    console_printf("%s is a block device.\n", filename);
                    break;

                case DIR_ENTRY_TYPE_UNKNOWN:
                    console_printf("Unknown file type %s.\n", filename);
                    break;

                case DIR_ENTRY_TYPE_UNUSED:
                    console_printf("File %s not found.\n", filename);
                    break;
            }
        } else if (!strcmp(command, "pwd")) {
            // TODO
        } else if (!strcmp(command, "help")) {
            console_puts(
                "Help:\n"
                "cat        - prints out the contents of a file\n"
                "cd         - changes the current directory\n"
                "hexdump    - prints out a hexdump of the specified file\n"
                "ls         - lists the current directory's contents\n"
                "pwd        - prints out the current working directory\n"
            );
        } else {
            console_printf("Unknown command %s. Use `help` to get a list of valid commands.\n", command);
        }

        free(s);
    }
}

