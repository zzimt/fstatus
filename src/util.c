#include "util.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

FILE* file_open(const char* path, const char* flags) {
    FILE* file = fopen(path, flags);
    if (file == NULL) {
        show_error_errno("failed to open `%s`", path);
        return NULL;
    }
    return file;
}

int file_open_and_scan(const char* path, const char* format, ...) {
    FILE* file = file_open(path, "r");
    if (!file) return EOF;

    va_list args;
    va_start(args, format);

    int res = vfscanf(file, format, args);
    if (res == EOF) {
        show_error_errno("failed to read `%s`", path);
        return EOF;
    }

    va_end(args);

    fclose(file);

    return res;
}

bool file_open_and_get_line(const char* path, char* buf, size_t size) {
    FILE* file = file_open(path, "r");
    if (!file) return false;

    if (fgets(buf, size, file) == NULL) {
        show_error_errno("failed to read `%s`", path);
        return false;
    }

    fclose(file);

    return true;
}

void show_error(const char* format, ...) {
    va_list args;
    va_start(args, format);

    fprintf(stderr, "error: ");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");

    va_end(args);
}

void show_error_errno(const char* format, ...) {
    va_list args;
    va_start(args, format);

    fprintf(stderr, "error: ");
    vfprintf(stderr, format, args);
    fprintf(stderr, ": %s", strerror(errno));
    fprintf(stderr, "\n");

    va_end(args);
}

size_t string_print(char* buf, size_t size, const char* format, ...) {
    va_list args;
    va_start(args, format);

    int needed = vsnprintf(buf, size, format, args);

    va_end(args);

    if (needed < 0) {
        return 0;
    }

    if ((size_t)needed >= size && size > 0) {
        return size - 1;
    } else {
        return needed;
    }
}

char* string_trim(char* str) {
    while (isspace(*str) && *str != '\0') {
        str++;
    }

    size_t len = strlen(str);
    while (len > 0 && isspace(str[len - 1])) {
        len--;
    }

    str[len] = '\0';

    return str;
}
