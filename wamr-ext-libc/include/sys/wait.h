#ifndef	_SYS_WAIT_H
#define	_SYS_WAIT_H
#ifdef __cplusplus
extern "C" {
#endif

#include <features.h>

#define __NEED_pid_t
#define __NEED_id_t
#include <bits/alltypes.h>

pid_t wait (int *);
pid_t waitpid (pid_t, int *, int );

#define WNOHANG    1
#define WUNTRACED  2

#define WSTOPPED   2
#define WEXITED    4
#define WCONTINUED 8
#define WNOWAIT    0x1000000

#define __WNOTHREAD 0x20000000
#define __WALL      0x40000000
#define __WCLONE    0x80000000

#define WEXITSTATUS(s) (s)
#define WTERMSIG(s) 0
#define WSTOPSIG(s) 0
#define WCOREDUMP(s) 0
#define WIFEXITED(s) (!WEXITSTATUS(s))
#define WIFSTOPPED(s) (0)
#define WIFSIGNALED(s) (0)
#define WIFCONTINUED(s) (0)

#ifdef __cplusplus
}
#endif
#endif
