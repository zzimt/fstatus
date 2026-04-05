#ifndef FSTATUS_MODULES_CPU_H_
#define FSTATUS_MODULES_CPU_H_

#include <stddef.h>
#include <stdbool.h>

typedef struct {
    int decimal_places;
    int spacing;
    bool left_align;
    double min_update_interval_seconds;
} CpuConfig;

#define Cpu_DEFAULT_CONFIG \
    .decimal_places = 0, \
    .spacing = 0, \
    .left_align = false, \
    .min_update_interval_seconds = 0.99, \

size_t Cpu_update(const void* config, char* buf, size_t size);

#endif // FSTATUS_MODULES_CPU_H_
