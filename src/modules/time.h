#ifndef FSTATUS_MODULES_TIME_H_
#define FSTATUS_MODULES_TIME_H_

#include <stddef.h>

typedef struct {
    const char* format;
} TimeConfig;

#define Time_DEFAULT_CONFIG \
    .format = "%a %d %b %H:%M",

size_t Time_update(const void* config, char* buf, size_t size);

#endif // FSTATUS_MODULES_TIME_H_
