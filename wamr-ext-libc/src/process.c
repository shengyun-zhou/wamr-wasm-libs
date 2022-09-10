#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <spawn.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>

#define hidden
#include <stdio_impl.h>

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

int posix_spawn(pid_t *restrict pid, const char *restrict path,
                const posix_spawn_file_actions_t *restrict file_actions,
                const posix_spawnattr_t *restrict attrp,
                char *const argv[restrict],
                char *const envp[restrict]) {
    // Don't support command path now
    return posix_spawnp(pid, path, file_actions, attrp, argv, envp);
}

int posix_spawnp(pid_t *restrict pid, const char *restrict file,
                 const posix_spawn_file_actions_t *restrict file_actions,
                 const posix_spawnattr_t *restrict attrp,
                 char *const argv[restrict],
                 char *const envp[restrict]) {
    if (!file || !(file[0]))
        return EINVAL;
    return ENOSYS;
}


pid_t wait(int *stat_loc) {
    return waitpid(-1, stat_loc, 0);
}

pid_t waitpid(pid_t pid, int *stat_loc, int options) {
    errno = ENOSYS;
    return -1;
}

