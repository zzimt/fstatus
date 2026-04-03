#ifndef FSTATUS_UTIL_H
#define FSTATUS_UTIL_H

#include <stddef.h>
#include <stdbool.h>
#include <time.h>

#define ARRLEN(arr_) (sizeof(arr_) / sizeof(arr_[0]))

typedef enum Ordering {
    OLESS,
    OEQUAL,
    OGREATER,
} Ordering;

int file_open_and_scan(const char* path, const char* format, ...);

bool file_open_and_get_line(const char* path, char* buf, size_t size);

void show_error(const char* format, ...);

void show_error_errno(const char* format, ...);

size_t string_print(char* buf, size_t size, const char* format, ...);

char* string_trim(char* str);

typedef struct Stopwatch {
    struct timespec ts_from;
    struct timespec ts_now;
} Stopwatch;

void stopwatch_update(Stopwatch* stopwatch);

bool stopwatch_check(const Stopwatch* stopwatch, const struct timespec* ts);

bool stopwatch_check_double(const Stopwatch* stopwatch, double seconds);

void stopwatch_reset(Stopwatch* stopwatch);

struct timespec timespec_diff(const struct timespec* l, const struct timespec* r);

Ordering timespec_cmp(const struct timespec* l, const struct timespec* r);

double timespec_to_double(const struct timespec* ts);

struct timespec timespec_from_double(double seconds);

#endif // FSTATUS_UTIL_H
