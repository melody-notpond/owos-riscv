#include "drivers/filesystems/generic_file.h"
#include "drivers/uart/uart.h"
#include "kshell.h"
#include "lib/memory.h"
#include "lib/string.h"

#define COMMAND_SIZE 16

// kshell_main(generic_dir_t*) -> void
// Does the shell.
void kshell_main(generic_dir_t* root) {
    generic_dir_t* current = root;

    while (1) {
        char* s = uart_readline("> ");

        char command[COMMAND_SIZE] = { 0 };
        int i;
        for (i = 0; i < COMMAND_SIZE && s[i]; i++) {
            if (s[i] == ' ')
                break;
            else
                command[i] = s[i];
        }

        if (command[COMMAND_SIZE - 1] != 0) {
            uart_printf("Command %s is dummy thicc.\n", s);
            free(s);
            continue;
        }

        if (command[0] == 0) {
        } else if (!strcmp(command, "ls")) {
        } else if (!strcmp(command, "cd")) {
        } else if (!strcmp(command, "hexdump")) {
            char* filename = s + i + 1;
            struct s_dir_entry entry = generic_dir_lookup(current, filename);

            switch (entry.tag) {
                case DIR_ENTRY_TYPE_DIR: {
                    generic_dir_t* dir = entry.value.dir;
                    generic_file_t* file = (*dir)->value;
                    unsigned long long filesize = generic_file_size(file);
                    void* buffer = malloc(filesize);
                    generic_file_read(file, buffer, filesize);
                    uart_put_hexdump(buffer, filesize);
                    free(buffer);
                    break;
                }

                case DIR_ENTRY_TYPE_REGULAR: {
                    generic_file_t* file = entry.value.file;
                    unsigned long long filesize = generic_file_size(file);
                    void* buffer = malloc(filesize);
                    generic_file_read(file, buffer, filesize);
                    uart_put_hexdump(buffer, filesize);
                    free(buffer);
                    close_generic_file(file);
                    break;
                }

                case DIR_ENTRY_TYPE_BLOCK:
                    uart_printf("%s is a block device.\n", filename);
                    break;

                case DIR_ENTRY_TYPE_UNKNOWN:
                    uart_printf("Unknown file type %s.\n", filename);
                    break;

                case DIR_ENTRY_TYPE_UNUSED:
                    uart_printf("File %s not found.\n", filename);
                    break;
            }
        } else if (!strcmp(command, "cat")) {
        } else if (!strcmp(command, "help")) {
            uart_puts(
                "Help:\n"
                "cat        - prints out the contents of a file\n"
                "cd         - changes the current directory\n"
                "hexdump    - prints out a hexdump of the specified file\n"
                "ls         - lists the current directory's contents\n"
            );
        } else {
            uart_printf("Unknown command %s. Use `help` to get a list of valid commands.\n", command);
        }

        free(s);
    }
}

