#define _GNU_SOURCE

#include <asc.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <linux/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>

#ifndef PTRACE_O_EXITKILL
  #define PTRACE_O_EXITKILL (1 << 20)
#endif

#define OPTIONS (PTRACE_O_TRACEEXIT | PTRACE_O_EXITKILL)

int main(int argc, char *argv[])
{
    struct map *map;
    int status, pid;
    long a;

    switch (pid = fork()) {
    case 0:
        if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
            fprintf(stderr, "ptrace PTRACE_TRACEME: %m\n");
            __builtin_trap();
        }

        execvpe(argv[1], argv + 1, 0);

        fprintf(stderr, "execvpe: `%s': %m\n", argv[1]);

        exit(1);
    case -1:
        __builtin_trap();
    }

    if (waitpid(pid, &status, 0) == -1) {
        fprintf(stderr, "waitpid: %m\n");
        __builtin_trap();
    }

    if (WIFSTOPPED(status) != 1 ) {
        fprintf(stderr, "panic: child is not stopped\n");
        __builtin_trap();
    }

    if (ptrace(PTRACE_SETOPTIONS, pid, 0, OPTIONS) < 0) {
        fprintf(stderr, "ptrace PTRACE_O_SETOPTIONS: %m\n");
        __builtin_trap();
    }

    if ((map = maps(pid, 0)) == 0)
        __builtin_trap();

    for (a = 0; map; a += map->io.iov_len, map = map->hh.next) {
        printf("%8ld %8ld %16lx %16lx %7ld %18s\n",
               a,
               a + map->io.iov_len - 1,
               (long) map->io.iov_base,
               (long) map->io.iov_base + map->io.iov_len - 1,
               map->io.iov_len,
               map->name);
    }


    return 0;
}
