#include "customcmd.h"

#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "../util.h"

static Stopwatch stopwatch = { 0 };

static char cmd_output[256] = { 0 };

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

size_t CustomCmd_update(const void* config, char* buf, size_t size) {
    const CustomCmdConfig* cfg = (CustomCmdConfig*)config;

    stopwatch_update(&stopwatch);

    if (stopwatch_check_double(&stopwatch, cfg->min_update_interval_seconds)) {
        int fds[2] = { 0 };
        if (pipe(fds) < 0) {
            show_error_errno("failed to pipe");
            return -1;
        }

        char cmd_string[256];
        cmd_to_string(cfg->command, cmd_string, sizeof(cmd_string));

        pid_t pid = fork();

        if (pid < 0) {
            show_error_errno("failed to fork");
            return -1;
        }

        if (pid == 0) {
            const char* file = cfg->command[0];
            const char** args = cfg->command;

            close(fds[0]);
            dup2(fds[1], STDOUT_FILENO);
            close(fds[1]);

            execvp((char*) file, (char**) args);

            show_error_errno("failed to execute `%s`", cmd_string);
            exit(1);
        }
        close(fds[1]);

        int n = read(fds[0], cmd_output, sizeof(cmd_output));
        if (n < 0) {
            show_error_errno("failed to read from command `%s`", cmd_string);
            return 0;
        }

        close(fds[0]);

        cmd_output[n] = '\0';
        if (n > 0 && cmd_output[n - 1] == '\n') 
            cmd_output[n - 1] = '\0';
        
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) != 0) {
                show_error("command `%s` exited with status %d`", cmd_string, status);
                return 0;
            }
        }

        stopwatch_reset(&stopwatch);
    }

    return string_print(buf, size, "%s", cmd_output);
}
