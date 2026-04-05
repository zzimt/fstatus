#include "mem.h"

#include <stdint.h>

#include "../util.h"

size_t Mem_update(const void* config, char* buf, size_t size) {
    (void)config;

    uint64_t total;
    uint64_t free;
    uint64_t buffers;
    uint64_t cached;

    if (file_open_and_scan("/proc/meminfo",
        "MemTotal: %ju kB\n"
        "MemFree: %ju kB\n"
        "MemAvailable: %ju kB\n"
        "Buffers: %ju kB\n"
        "Cached: %ju kB\n",
        &total, &free, &buffers, &buffers, &cached) != 5) {
        return 0;
    }

    if (total == 0) {
        return 0;
    }

    int percent = 100 * ((total - free) - (buffers + cached)) / total;
    return string_print(buf, size, "%d%%", percent);
}
