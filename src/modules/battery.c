#include "battery.h"

#include <stdio.h>

#include "../util.h"

#define BATTERY_CAPACITY_FORMAT "/sys/class/power_supply/%s/capacity"

static int poll_capacity(const char* bat) {
    static char path[128];
    snprintf(path, sizeof(path), "/sys/class/power_supply/%s/capacity", bat);

    int cap;
    if (file_open_and_scan(path, "%d", &cap) == EOF) {
        return 0;
    }

    return cap;
}

static bool poll_charging(const char* adp) {
    static char path[128];
    snprintf(path, sizeof(path), "/sys/class/power_supply/%s/online", adp);

    int online;
    if (file_open_and_scan(path, "%d", &online) == EOF) {
        return false;
    }

    return online;
}

size_t Battery_update(const void* config, char* buf, size_t size) {
    const BatteryConfig* cfg = (BatteryConfig*)config;
    
    if (cfg->charging_format == NULL) {
        show_error("Battery: `charging_format` is NULL");
        return 0;
    }
    if (cfg->format == NULL) {
        show_error("Battery: `format` is NULL");
        return 0;
    }

    int cap = poll_capacity(cfg->battery_name);
    bool charging = poll_charging(cfg->adapter_name);

    const char* format;
    if (charging) {
        format = cfg->charging_format; 
    } else {
        format = cfg->format;
    }

    return string_print(buf, size, format, cap);

    return 0;
}
