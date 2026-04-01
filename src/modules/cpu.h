#ifndef FSTATUS_MODULES_CPU_H_
#define FSTATUS_MODULES_CPU_H_

#include <stddef.h>
#include <stdbool.h>

typedef struct {
    const char* stat_path;
    int decimal_places;
    int spacing;
    bool left_align;
} CpuConfig;

#define Cpu_DEFAULT_CONFIG \
    .stat_path = "/proc/stat", \
    .decimal_places = 0, \
    .spacing = 0, \
    .left_align = false,

size_t Cpu_update(const void* config, char* buf, size_t size);

#endif // FSTATUS_MODULES_CPU_H_
