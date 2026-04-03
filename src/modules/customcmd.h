#ifndef FSTATUS_MODULES_CUSTOMCMD_H_
#define FSTATUS_MODULES_CUSTOMCMD_H_

#include <stddef.h>

typedef struct {
    const char** command;
    double min_update_interval_seconds;
} CustomCmdConfig;

#define CustomCmd_DEFAULT_CONFIG \
    .command = NULL, \
    .min_update_interval_seconds = 0.0, \

size_t CustomCmd_update(const void* config, char* buf, size_t size);

#endif // FSTATUS_MODULES_CUSTOMCMD_H_
