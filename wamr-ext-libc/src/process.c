#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <spawn.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>
#include "../internal/wamr_ext_syscall.h"

#include "../../wasi-libc/libc-top-half/musl/src/process/fdop.h"
#undef malloc
#undef calloc
#undef realloc
#undef free
#define hidden
#include "../../wasi-libc/libc-top-half/musl/src/internal/stdio_impl.h"

extern char **__environ;

void __process_add_arg(char*** out_argv, int* out_argc, char* new_arg) {
    if (!new_arg || !(*new_arg))
        return;
    if ((*out_argc) % 5 == 0)
        *out_argv = realloc(*out_argv, sizeof(char*) * ((*out_argc) + 5 + 1));
    (*out_argv)[(*out_argc)++] = strdup(new_arg);
    (*out_argv)[(*out_argc)] = NULL;
}

void __process_free_argv(char** argv) {
    if (!argv)
        return;
    for (int i = 0; argv[i]; i++)
        free(argv[i]);
    free(argv);
}

void __process_split_cmdline(const char* cmd, char*** out_argv) {
    *out_argv = NULL;
    int argc = 0;
    char end_char = ' ';
    const size_t cmdlen = strlen(cmd);
    char* temp_strbuf = malloc(cmdlen + 1);
    if (!temp_strbuf)
        return;

    size_t temp_strbuf_len = 0;
    bool escaped = false;
    for (size_t cur_pos = 0; cur_pos < cmdlen; cur_pos++) {
        char c = cmd[cur_pos];
        if (escaped) {
            temp_strbuf[temp_strbuf_len++] = c;
            escaped = false;
            continue;
        }
        if (c == '\\' && end_char != '\'') {
            escaped = true;
            continue;
        }

        if ((c == '"' || c == '\'') && end_char == ' ') {
            end_char = c;
            continue;
        }
        if (c == end_char) {
            if (end_char == ' ') {
                temp_strbuf[temp_strbuf_len] = '\0';
                __process_add_arg(out_argv, &argc, temp_strbuf);
                temp_strbuf_len = 0;
            }
            end_char = ' ';
            continue;
        }
        temp_strbuf[temp_strbuf_len++] = c;
    }
    // Ignore unmatched " or '
    temp_strbuf[temp_strbuf_len] = '\0';
    __process_add_arg(out_argv, &argc, temp_strbuf);
    free(temp_strbuf);
}

// Implementation ref: musl-libc
FILE *popen(const char *cmd, const char *mode) {
    if (!cmd || !mode || !(cmd[0])) {
        errno = EINVAL;
        return NULL;
    }

    int p[2], op, e;
    pid_t pid;
    FILE *f;
    posix_spawn_file_actions_t fa;

    if (*mode == 'r') {
        op = 0;
    } else if (*mode == 'w') {
        op = 1;
    } else {
        errno = EINVAL;
        return NULL;
    }

    if (socketpair(AF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0, p) != 0)
        return NULL;

    f = fdopen(p[op], mode);
    if (!f) {
        close(p[0]);
        close(p[1]);
        return NULL;
    }

    e = ENOMEM;
    if (!posix_spawn_file_actions_init(&fa)) {
        if (!posix_spawn_file_actions_adddup2(&fa, p[1 - op], 1 - op)) {
            char** argv = NULL;
            __process_split_cmdline(cmd, &argv);
            if (argv && !(e = posix_spawnp(&pid, argv[0], &fa, NULL, argv, __environ))) {
                posix_spawn_file_actions_destroy(&fa);
                __process_free_argv(argv);
                f->pipe_pid = pid;
                close(p[1 - op]);
                return f;
            }
            __process_free_argv(argv);
        }
        posix_spawn_file_actions_destroy(&fa);
    }
    fclose(f);
    close(p[1 - op]);

    errno = e;
    return 0;
}

int pclose(FILE *f) {
    int status = 0, r;
    pid_t pid = f->pipe_pid;
    fclose(f);

    while ((r = waitpid(pid, &status, 0)) == -1 && errno == EINTR);
    if (r < 0)
        return r;
    return WEXITSTATUS(status);
}

// Implementation ref: musl-libc
int system(const char *cmd) {
    if (!cmd || !(cmd[0]))
        return 1;
    pid_t pid;
    char** argv = NULL;
    __process_split_cmdline(cmd, &argv);
    int err = ENOMEM;
    if (argv)
        err = posix_spawnp(&pid, argv[0], NULL, NULL, argv, __environ);
    __process_free_argv(argv);
    int exit_status = -1;
    if (!err) {
        while (waitpid(pid, &exit_status, 0) == -1 && errno == EINTR);
        exit_status = WEXITSTATUS(exit_status);
    } else if (err == ENOENT) {
        // Treat as "command not found" in shell.
        exit_status = 127;
        err = 0;
    }
    if (err) {
        errno = err;
        return -1;
    }
    return exit_status;
}

int posix_spawn(pid_t *restrict pid, const char *restrict path,
                const posix_spawn_file_actions_t *restrict file_actions,
                const posix_spawnattr_t *restrict attrp,
                char *const argv[restrict],
                char *const envp[restrict]) {
    // Don't support command path now
    return posix_spawnp(pid, path, file_actions, attrp, argv, envp);
}

#define __WAMR_WASI_SPAWN_ACTION_FDUP2 1

struct wamr_spawn_action_base {
    struct wamr_spawn_action_base* next_action;
    uint32_t cmd;
};

struct wamr_spawn_action_fdup2 {
    struct wamr_spawn_action_base action_base;
    int32_t src_fd;
    int32_t dest_fd;
};

struct wamr_spawn_req {
    const char* cmd_name;
    char** argv;
    uint32_t argc;
    struct wamr_spawn_action_base* fa;
    int32_t ret_pid;
};

int posix_spawnp(pid_t *restrict pid, const char *restrict file,
                 const posix_spawn_file_actions_t *restrict file_actions,
                 const posix_spawnattr_t *restrict attrp,
                 char *const exec_argv[restrict],
                 char *const envp[restrict]) {
    if (!file || !(file[0]))
        return EINVAL;
    struct wamr_spawn_req req = {0};
    req.cmd_name = file;
    if (exec_argv && exec_argv[0]) {
        // Ignore first argument
        req.argv = (char**)&exec_argv[1];
        while (req.argv[req.argc])
            req.argc++;
    }
    if (file_actions && file_actions->__actions) {
        for (struct fdop* op = file_actions->__actions; op; op = op->next) {
            struct wamr_spawn_action_base* new_action_node = NULL;
            switch(op->cmd) {
                case FDOP_DUP2: {
                    struct wamr_spawn_action_fdup2* fa_dup2 = malloc(sizeof(struct wamr_spawn_action_fdup2));
                    new_action_node = (struct wamr_spawn_action_base*)fa_dup2;
                    fa_dup2->action_base.cmd = __WAMR_WASI_SPAWN_ACTION_FDUP2;
                    fa_dup2->src_fd = op->srcfd;
                    fa_dup2->dest_fd = op->fd;
                    break;
                }
            }
            if (new_action_node) {
                new_action_node->next_action = req.fa;
                req.fa = new_action_node;
            }
        }
    }
    wamr_ext_syscall_arg syscall_argv[] = {
        {.p = &req},
    };
    int32_t err = __imported_wamr_ext_syscall(__EXT_SYSCALL_PROC_SPAWN, sizeof(syscall_argv) / sizeof(wamr_ext_syscall_arg), syscall_argv);
    while (req.fa) {
        struct wamr_spawn_action_base* next_node = req.fa->next_action;
        free(req.fa);
        req.fa = next_node;
    }
    if (err == 0 && pid)
        *pid = req.ret_pid;
    return err;
}

pid_t wait(int *stat_loc) {
    return waitpid(-1, stat_loc, 0);
}

pid_t waitpid(pid_t pid, int *stat_loc, int options) {
    if (!stat_loc) {
        errno = EINVAL;
        return -1;
    }
    int32_t exit_status = UINT8_MAX;
    int32_t ret_pid = 0;
    wamr_ext_syscall_arg syscall_argv[] = {
        {.u32 = pid},
        {.u32 = options},
        {.p = &exit_status},
        {.p = &ret_pid}
    };
    int32_t err = __imported_wamr_ext_syscall(__EXT_SYSCALL_PROC_WAIT_PID, sizeof(syscall_argv) / sizeof(wamr_ext_syscall_arg), syscall_argv);
    if (err != 0) {
        errno = err;
        return -1;
    }
    if (ret_pid > 0)
        *stat_loc = exit_status;
    return ret_pid;
}

