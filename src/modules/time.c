#include "time.h"

#include <time.h>

size_t Time_update(const void* config, char* buf, size_t size) {
    const TimeConfig* cfg = (TimeConfig*)config;
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    return strftime(buf, size, cfg->format, tm);
}
