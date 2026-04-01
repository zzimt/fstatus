#ifndef FSTATUS_UTIL_H
#define FSTATUS_UTIL_H

#include <stddef.h>
#include <stdbool.h>

#define ARRLEN(arr_) (sizeof(arr_) / sizeof(arr_[0]))

int file_open_and_scan(const char* path, const char* format, ...);

bool file_open_and_get_line(const char* path, char* buf, size_t size);

void show_error(const char* format, ...);

void show_error_errno(const char* format, ...);

size_t string_print(char* buf, size_t size, const char* format, ...);

char* string_trim(char* str);

#endif // FSTATUS_UTIL_H
