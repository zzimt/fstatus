#include "temperature.h"

#include <stdio.h>

#include "../util.h"

static Stopwatch stopwatch = { 0 };
float temp = 0.0f;

static float poll_temp(const char* thermal_zone) {
    int temp;
    char path[128];
    snprintf(path, sizeof(path), "/sys/class/thermal/%s/temp", thermal_zone);

    if (file_open_and_scan(path, "%d", &temp) == EOF)
        return 0.0f;
    
    return temp;
}

size_t Temperature_update(const void* config, char* buf, size_t size) {
    const TemperatureConfig* cfg = (TemperatureConfig*)config;

    if (cfg->thermal_zone == NULL) {
        show_error("Temperature: `thermal_zone` is NULL");
        return 0;
    }

    stopwatch_update(&stopwatch);

    if (stopwatch_check_double(&stopwatch, cfg->min_update_interval_seconds)) {
        temp = poll_temp(cfg->thermal_zone);

        stopwatch_reset(&stopwatch);
    }

    const char* fmt = cfg->left_align ? "%-*.*f" : "%*.*f";
    return snprintf(buf, size, fmt, cfg->spacing, cfg->decimal_places, temp / cfg->divide_by);
}
