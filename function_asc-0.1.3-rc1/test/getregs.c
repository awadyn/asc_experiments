#define _GNU_SOURCE

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
    struct user_regs_struct gpr;
    int c, status, pid;
    long r;

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

    for (r = 0; ; r++) {

        if ((c = ptrace(PTRACE_GETREGS, pid, 0, &gpr)) < 0) {
            fprintf(stderr, "ptrace PTRACE_GETREGSET: %m\n");
            __builtin_trap();
        }

        if (errno) {
            fprintf(stderr, "ptrace PTRACE_GETREGSET: %m\n");
            __builtin_trap();
        }

        printf("%7ld\t%06lx\t%7ld\n", r, (long) gpr.rip, (long) gpr.rcx);

        if ((long) gpr.orig_rax != -1 && r > 0)
            break;

        if (ptrace(PTRACE_SYSEMU_SINGLESTEP, pid, 0, 0) < 0) {
            fprintf(stderr, "ptrace PTRACE_SYSEMU_SINGLESTEP: %m\n");
            __builtin_trap();
        }

        if (waitpid(pid, &status, 0) == -1) {
            fprintf(stderr, "waitpid: %m\n");
            __builtin_trap();
        }
    }

    return 0;
}
