#ifndef FSTATUS_MODULES_CUSTOMCMD_H_
#define FSTATUS_MODULES_CUSTOMCMD_H_

#include <stddef.h>

typedef struct {
    const char** command;
} CustomCmdConfig;

#define CustomCmd_DEFAULT_CONFIG \
    .command = NULL,

size_t CustomCmd_update(const void* config, char* buf, size_t size);

#endif // FSTATUS_MODULES_CUSTOMCMD_H_
