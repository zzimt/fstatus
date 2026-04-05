#ifndef FSTATUS_MODULES_VOLUME_H_
#define FSTATUS_MODULES_VOLUME_H_

#include <stddef.h>

typedef struct {
    const char* muted_text;
} VolumeConfig;

#define Volume_DEFAULT_CONFIG \
    .muted_text = "muted",

size_t Volume_update(const void* config, char* buf, size_t size);

#endif // FSTATUS_MODULES_VOLUME_H_
