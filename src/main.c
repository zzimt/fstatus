#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/select.h>

#include <X11/Xlib.h>

#include "config.h"
#include "util.h"

static const char* program_name;

static void get_pid_path(char* dest, size_t size) {
    uid_t euid = geteuid();
    snprintf(dest, size, "/run/user/%d/fstatus.pid", (int)euid);
}

static bool save_pid(void) {
    char pid_path[256];
    get_pid_path(pid_path, sizeof(pid_path));

    bool result = true;

    FILE* file = fopen(pid_path, "w");
    if (file == NULL) {
        show_error_errno("failed to save pid to `%s`", pid_path);
        result = false;
        goto cleanup;
    }
    if (fprintf(file, "%d", (int)getpid()) < 0) {
        show_error_errno("failed to save pid to `%s`", pid_path);
        result = false;
        goto cleanup;
    }

cleanup:
    if (file != NULL) {
        fclose(file);
    }
    return result;
}

static bool load_pid(pid_t* pid) {
    char pid_path[256];
    get_pid_path(pid_path, sizeof(pid_path));

    bool result = false;

    FILE* file = fopen(pid_path, "r");
    if (file == NULL) {
        show_error_errno("failed to open pid file `%s`", pid_path);
        goto cleanup;
    }

    int ipid;
    if (fscanf(file, "%d", &ipid) != 1) {
        show_error_errno("failed to read pid file `%s`", pid_path);
        goto cleanup;
    }

    if (kill(ipid, 0) != 0) {
        show_error("stale pid in file `%s`", pid_path);
        goto cleanup;
    }

    *pid = (pid_t)ipid;

    result = true;

cleanup:
    if (file != NULL) {
        fclose(file);
    }

    return result;
}

static bool display_status(bool stdout_flag, Display* display) {
    static char buf[1024];
    char* bufptr = buf;
    size_t bufsz = sizeof(buf);
    for (size_t i = 0; i < ARRLEN(status_format); i++) {
        const Format* fmt = status_format + i;

        size_t len = 0;
        switch (fmt->kind) {
        case FORMAT_TEXT:
            len = string_print(bufptr, bufsz, "%s", fmt->val.text.value);
            break;
        case FORMAT_MODULE: 
            len = fmt->val.module.update_fn(fmt->val.module.config, bufptr, bufsz);
            break;
        }

        bufptr += len;
        bufsz -= len;
    }

    if (stdout_flag) {
        printf("%s\n", buf);
    } else {
        if (XStoreName(display, DefaultRootWindow(display), buf) < 0) {
            show_error("failed to store status string\n");
            return false;
        }
        XFlush(display);
    }

    return true;
}

static int run_update_request(void) {
    pid_t pid;
    if (!load_pid(&pid)) {
        return 1;
    }

    if (kill(pid, SIGUSR1) != 0) {
        show_error_errno("failed to request update");
        return 1;
    }

    return 0;
}

static int run_kill(void) {
    pid_t pid;
    if (!load_pid(&pid)) {
        return 1;
    }

    if (kill(pid, SIGUSR2) != 0) {
        show_error_errno("failed to kill");
        return 1;
    }

    return 0;
}

static int start_periodic_update_requests(int kill_fd) {
    pid_t pid;
    if (!load_pid(&pid)) {
        return 1;
    }

    const struct timespec ts = timespec_from_double(status_update_interval_seconds);

    while (true) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(kill_fd, &fds);

        int r = pselect(kill_fd + 1, &fds, NULL, NULL, &ts, NULL);
        if (r < 0) {
            if (errno == EINTR) continue;
            show_error_errno("select failed");
            return 1;
        }

        if (FD_ISSET(kill_fd, &fds)) break;

        if (kill(pid, SIGUSR1) != 0) {
            show_error_errno("failed to request update");
            return 1;
        }
    }

    return 0;
}

static volatile sig_atomic_t run_normal_done = 0;

static void run_normal_handle_sigusr2(int sig) {
    (void)sig;
    run_normal_done = 1;
}

static int run_normal(bool stdout_flag) {
    if (!save_pid()) {
        return 1;
    }

    struct sigaction sa;
    sa.sa_handler = run_normal_handle_sigusr2;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGUSR2, &sa, NULL) < 0) {
        show_error_errno("failed to set SIGUSR2 handler");
    }

    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &sigset, NULL) != 0) {
        show_error_errno("failed to block signals");
        return 1;
    }

    int kill_child_pipe_fds[2];
    if (pipe(kill_child_pipe_fds) < 0) {
        show_error_errno("failed to pipe");
        return 1;
    }

    int child_pid = fork();
    if (child_pid < 0) {
        show_error_errno("failed to fork");
        return 1;
    }

    if (child_pid == 0) {
        close(kill_child_pipe_fds[1]);
        return start_periodic_update_requests(kill_child_pipe_fds[0]);
    }

    close(kill_child_pipe_fds[0]);

    int result = 1;

    Display* display = NULL;

    if (!stdout_flag) {
        display = XOpenDisplay(NULL);
        if (display == NULL) {
            show_error("failed to open X11 display");
            goto cleanup;
        }
    }

    if (!display_status(stdout_flag, display)) {
        goto cleanup;
    }

    while (!run_normal_done) {
        int sig;
        int err = sigwait(&sigset, &sig);
        if (err != 0) {
            errno = err;
            show_error_errno("failed to sigwait");
            goto cleanup;
        }

        if (sig == SIGUSR1) {
            if (!display_status(stdout_flag, display)) {
                goto cleanup;
            }
        }
    }

    result = 0;

cleanup:
    if (display) XCloseDisplay(display);

    close(kill_child_pipe_fds[1]);

    int child_status;
    for (int i = 0; i < 5; i++) {
        pid_t r = waitpid(child_pid, &child_status, WNOHANG);
        if (r == child_pid) {
            return result;
        }
        sleep(1);
    }

    kill(child_pid, SIGKILL);
    waitpid(child_pid, &child_status, 0);
    return result;
}

static void print_version(void) {
    printf("fstatus v"VERSION"\n");
}

static void print_help(void) {
    print_version();
    printf("USAGE:\n");
    printf("    %s [OPTION]\n", program_name);
    printf("OPTIONS:\n");
    printf("    -r, --request-update - request immediate update\n");
    printf("    -h, --help           - print help message and exit\n");
    printf("    -v, --version        - print version info and exit\n");
    printf("    -s, --stdout         - only output to stdout\n");
    printf("    -k, --kill           - kill active fstatus process\n");
}

static char* adv_args(int* argc, char*** argv) {
    if (*argc == 0) return NULL;
    char* arg = **argv;
    (*argc)--;
    (*argv)++;
    return arg;
}

static bool is_arg(char* arg, const char* arg_short, const char* arg_long) {
    return (strcmp(arg, arg_short) == 0 || strcmp(arg, arg_long) == 0);
}

int main(int argc, char** argv) {
    program_name = adv_args(&argc, &argv);
    bool help_flag = false;
    bool version_flag = false;
    bool request_update_flag = false;
    bool stdout_flag = false;
    bool kill_flag = false;
    while (true) {
        char* arg = adv_args(&argc, &argv);
        if (arg == NULL) break;
        if (is_arg(arg, "-h", "--help")) {
            help_flag = true;
        } else if (is_arg(arg, "-v", "--version")) {
            version_flag = true;
        } else if (is_arg(arg, "-r", "--request-update")) {
            request_update_flag = true;
        } else if (is_arg(arg, "-s", "--stdout")) {
            stdout_flag = true;
        } else if (is_arg(arg, "-k", "--kill")) {
            kill_flag = true;
        } else {
            show_error("invalid argument `%s`", arg);
            fprintf(stderr, "use `%s --help` for usage\n", program_name);
            return 1;
        }
    }

    if (help_flag) {
        print_help();
        return 0;
    }
    if (version_flag) {
        print_version();
        return 0;
    }
    if (request_update_flag) {
        return run_update_request();
    }
    if (kill_flag) {
        return run_kill();
    }
    return run_normal(stdout_flag);
}
