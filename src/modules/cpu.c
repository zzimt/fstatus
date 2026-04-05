#include "cpu.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../util.h"

typedef struct CpuState {
    uint64_t total;
    uint64_t work;
} CpuState;

static Stopwatch stopwatch = { 0 };

static CpuState cpu_curr = { 0 };
static CpuState cpu_prev = { 0 };
float cpu_load = 0.0f;

typedef enum StatField {
    STAT_USER = 0,
    STAT_NICE,
    STAT_SYSTEM,
    STAT_IDLE,
    STAT_IOWAIT,
    STAT_IRQ,
    STAT_SOFTIRQ,
    STAT_STEAL,
    STAT_GUEST,
    STAT_GUEST_NICE,
    STAT_MAXCOUNT,
} StatField;

static bool parse_uint64(const char *str, uint64_t *out) {
    errno = 0;

    char *endptr;
    unsigned long long val = strtoull(str, &endptr, 10);

    if (errno == ERANGE || val > UINT64_MAX) {
        return false;
    }
    if (endptr == str) {
        return false;
    }

    *out = (uint64_t)val;
    return true; 
}

static float poll_cpu(const char* stat_path) {
    static char line[256];
    if (!file_open_and_get_line(stat_path, line, sizeof(line))) {
        return 0.0f;
    }

    char* first = strtok(line, " ");
    if (first == NULL) {
        fprintf(stderr, "error: `%s` has invalid format\n", stat_path);
        return 0.0f;
    }

    uint64_t fields[STAT_MAXCOUNT] = { 0 };
    size_t field_count = 0;
    for (StatField field = 0; field < STAT_MAXCOUNT; field++, field_count++) {
        char* sfield = strtok(NULL, " ");
        if (sfield == NULL) {
            break;
        }
        uint64_t ufield;
        if (!parse_uint64(sfield, &ufield)) {
            fprintf(stderr, "error: `/proc/stat` has invalid format\n");
            return 0.0f;
        }
        fields[field] = ufield;
    }

    uint64_t idle = fields[STAT_IDLE] + fields[STAT_IOWAIT];
    uint64_t non_idle = 
        fields[STAT_USER] +
        fields[STAT_NICE] +
        fields[STAT_SYSTEM] +
        fields[STAT_IRQ] +
        fields[STAT_SOFTIRQ] +
        fields[STAT_STEAL] +
        fields[STAT_GUEST] +
        fields[STAT_GUEST_NICE];
    uint64_t total = idle + non_idle;

    cpu_prev = cpu_curr;
    cpu_curr.total = total;
    cpu_curr.work = non_idle;

    return (float)(cpu_curr.work - cpu_prev.work) / 
           (float)(cpu_curr.total - cpu_prev.total);
}

size_t Cpu_update(const void* config, char* buf, size_t size) {
    const CpuConfig* cfg = (CpuConfig*)config;

    stopwatch_update(&stopwatch);

    if (stopwatch_check_double(&stopwatch, cfg->min_update_interval_seconds)) {
        cpu_load = poll_cpu("/proc/stat");

        stopwatch_reset(&stopwatch);
    }

    const char* fmt = cfg->left_align ? "%-*.*f%%" : "%*.*f%%";
    return string_print(buf, size, fmt, cfg->spacing, cfg->decimal_places, cpu_load * 100.0f);
}
