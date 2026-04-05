#include "util.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

FILE* file_open(const char* path, const char* flags) {
    FILE* file = fopen(path, flags);
    if (file == NULL) {
        show_error_errno("failed to open `%s`", path);
        return NULL;
    }
    return file;
}

int file_open_and_scan(const char* path, const char* format, ...) {
    FILE* file = file_open(path, "r");
    if (!file) return EOF;

    va_list args;
    va_start(args, format);

    int res = vfscanf(file, format, args);
    if (res == EOF) {
        show_error_errno("failed to read `%s`", path);
        return EOF;
    }

    va_end(args);

    fclose(file);

    return res;
}

bool file_open_and_get_line(const char* path, char* buf, size_t size) {
    FILE* file = file_open(path, "r");
    if (!file) return false;

    if (fgets(buf, size, file) == NULL) {
        show_error_errno("failed to read `%s`", path);
        return false;
    }

    fclose(file);

    return true;
}

void show_error(const char* format, ...) {
    va_list args;
    va_start(args, format);

    fprintf(stderr, "error: ");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");

    va_end(args);
}

void show_error_errno(const char* format, ...) {
    va_list args;
    va_start(args, format);

    fprintf(stderr, "error: ");
    vfprintf(stderr, format, args);
    fprintf(stderr, ": %s", strerror(errno));
    fprintf(stderr, "\n");

    va_end(args);
}

size_t string_print(char* buf, size_t size, const char* format, ...) {
    va_list args;
    va_start(args, format);

    int needed = vsnprintf(buf, size, format, args);

    va_end(args);

    if (needed < 0) {
        return 0;
    }

    if ((size_t)needed >= size && size > 0) {
        return size - 1;
    } else {
        return needed;
    }
}

char* string_trim(char* str) {
    while (isspace(*str) && *str != '\0') {
        str++;
    }

    size_t len = strlen(str);
    while (len > 0 && isspace(str[len - 1])) {
        len--;
    }

    str[len] = '\0';

    return str;
}

void stopwatch_update(Stopwatch* stopwatch) {
    clock_gettime(CLOCK_MONOTONIC, &stopwatch->ts_now);
}

bool stopwatch_check(const Stopwatch* stopwatch, const struct timespec* ts) {
    struct timespec diff = timespec_diff(&stopwatch->ts_now, &stopwatch->ts_from);
    Ordering ord = timespec_cmp(&diff, ts);
    if (ord == OGREATER || ord == OEQUAL) return true;
    return false;
}

bool stopwatch_check_double(const Stopwatch* stopwatch, double seconds) {
    struct timespec ts = timespec_from_double(seconds);
    return stopwatch_check(stopwatch, &ts);
}

void stopwatch_reset(Stopwatch* stopwatch) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    stopwatch->ts_from = now;
    stopwatch->ts_now = now;
}

struct timespec timespec_diff(const struct timespec* l, const struct timespec* r) {
    struct timespec diff = {
        .tv_sec = l->tv_sec - r->tv_sec,
        .tv_nsec = l->tv_nsec - r->tv_nsec
    };
    if (diff.tv_nsec < 0) {
        diff.tv_nsec += 1000000000;
        diff.tv_sec--;
    }
    return diff;
}

Ordering timespec_cmp(const struct timespec* l, const struct timespec* r) {
    if (l->tv_sec == r->tv_sec) {
        if (l->tv_nsec == r->tv_nsec) {
            return OEQUAL;
        } else if (l->tv_nsec > r->tv_nsec) {
            return OGREATER;
        } else {
            return OLESS;
        }
    } else if (l->tv_sec > r->tv_sec) {
        return OGREATER;
    } else {
        return OLESS;
    }
}

double timespec_to_double(const struct timespec* ts) {
    return (double)ts->tv_sec 
         + (double)ts->tv_nsec / 1e9;
}

struct timespec timespec_from_double(double seconds) {
    struct timespec ts = {
        .tv_sec = (time_t)seconds,
        .tv_nsec = (time_t)((seconds - (long)seconds) * 1e9)
    };
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_nsec -= 1000000000;
        ts.tv_sec++;
    }
    return ts;
}

static void cmd_to_string(const char** cmd, char* buf, size_t size) {
    char* bufptr = buf;
    size_t bufsz = size;
    for (; *cmd != NULL; cmd++) {
        size_t n = string_print(bufptr, bufsz, "%s%s", *cmd, cmd[1] ? " " : "");
        bufptr += n;
        bufsz -= n;
        if (bufsz == 0) break;

    }
}

bool read_command_output(const char** cmd, char* buf, size_t size) {
        int fds[2] = { 0 };
        if (pipe(fds) < 0) {
            show_error_errno("failed to pipe");
            return false;
        }

        char cmd_string[256];
        cmd_to_string(cmd, cmd_string, sizeof(cmd_string));

        pid_t pid = fork();

        if (pid < 0) {
            show_error_errno("failed to fork");
            return false;
        }

        if (pid == 0) {
            const char* file = cmd[0];
            const char** args = cmd;

            close(fds[0]);
            dup2(fds[1], STDOUT_FILENO);
            close(fds[1]);

            execvp((char*) file, (char**) args);

            show_error_errno("failed to execute `%s`", cmd_string);
            exit(1);
        }
        close(fds[1]);

        int n = read(fds[0], buf, size);
        if (n < 0) {
            show_error_errno("failed to read from command `%s`", cmd_string);
            return false;
        }

        close(fds[0]);

        buf[n] = '\0';
        string_trim(buf);
        
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) != 0) {
                show_error("command `%s` exited with status %d`", cmd_string, status);
                return false;
            }
        }

        return true;
}
