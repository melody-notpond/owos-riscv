#ifndef KERNEL_STRING_H
#define KERNEL_STRING_H

// strcmp(const char*, const char*) -> int
// Returns 0 if the strings are equal, 1 if the first string is greater than the second, and -1 otherwise.
int strcmp(const char* s1, const char* s2);

// strlen(const char*) -> unsigned long int
// Calculates the length of a string (not including null terminator).
unsigned long int strlen(const char* s);

// strdup(const char*) -> char*
// Duplicates a string.
char* strdup(const char* s);

#endif /* KERNEL_STRING_H */

