#ifndef FSTATUS_MODULES_BATTERY_H_
#define FSTATUS_MODULES_BATTERY_H_

#include <stddef.h>

typedef struct {
    const char* battery_name;
    const char* adapter_name;
    const char* format;
    const char* charging_format;
} BatteryConfig; 

#define Battery_DEFAULT_CONFIG \
    .battery_name = NULL, \
    .adapter_name = NULL, \
    .format = "%d%%", \
    .charging_format = "%d%%+", \

size_t Battery_update(const void* config, char* buf, size_t size);

#endif // FSTATUS_MODULES_BATTERY_H_
