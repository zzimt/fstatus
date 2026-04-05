#include "customcmd.h"

#include "../util.h"

static Stopwatch stopwatch = { 0 };

static char cmd_output[256] = { 0 };

size_t CustomCmd_update(const void* config, char* buf, size_t size) {
    const CustomCmdConfig* cfg = (CustomCmdConfig*)config;

    stopwatch_update(&stopwatch);

    if (stopwatch_check_double(&stopwatch, cfg->min_update_interval_seconds)) {
        if (!read_command_output(cfg->command, cmd_output, sizeof(cmd_output))) {
            return 0; 
        }
        stopwatch_reset(&stopwatch);
    }

    return string_print(buf, size, "%s", cmd_output);
}
