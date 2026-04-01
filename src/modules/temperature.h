#ifndef FSTATUS_MODULES_TEMPERATURE_H_
#define FSTATUS_MODULES_TEMPERATURE_H_

#include <stddef.h>
#include <stdbool.h>

typedef struct {
    const char* thermal_zone;
    int divide_by;
    int decimal_places;
    int spacing;
    bool left_align;
} TemperatureConfig;

#define Temperature_DEFAULT_CONFIG \
    .thermal_zone = NULL, \
    .divide_by = 1000, \
    .decimal_places = 0, \
    .spacing = 0, \
    .left_align = false,

size_t Temperature_update(const void* config, char* buf, size_t size);

#endif // FSTATUS_MODULES_TEMPERATURE_H_
