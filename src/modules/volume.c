#include "volume.h"

#include <string.h>
#include <stdbool.h>

#include "../util.h"

size_t Volume_update(const void* config, char* buf, size_t size) {
    const VolumeConfig* cfg = (VolumeConfig*)config;

    static const char* muted_cmd[] = {
        "/bin/sh", "-c",
        "pactl get-sink-mute @DEFAULT_SINK@ | awk '{print $2}'",
        NULL,
    };

    static const char* volume_cmd[] = {
        "/bin/sh", "-c",
        "pactl get-sink-volume @DEFAULT_SINK@ | awk 'NR==1{print $5}'",
        NULL,
    };

    char muted_output[64] = { 0 };
    if (!read_command_output(muted_cmd, muted_output, sizeof(muted_output))) {
        return 0;
    }

    bool muted;
    if (strcmp(muted_output, "yes") == 0) {
        muted = true;
    } else {
        muted = false;
    }

    if (muted) {
        return string_print(buf, size, cfg->muted_text);
    }

    char volume_output[64] = { 0 };
    if (!read_command_output(volume_cmd, volume_output, sizeof(volume_output))) {
        return 0;
    }
    return string_print(buf, size, "%s", volume_output);
}
