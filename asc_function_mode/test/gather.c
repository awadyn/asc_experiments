#define _GNU_SOURCE

#include <asc.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <linux/ptrace.h>
#include <sys/syscall.h>
#include <sys/user.h>
#include <sys/wait.h>

#ifndef PTRACE_O_EXITKILL
  #define PTRACE_O_EXITKILL (1 << 20)
#endif

#define OPTIONS (PTRACE_O_TRACEEXIT | PTRACE_O_EXITKILL)

int main(int argc, char *argv[])
{
    struct map *map;
    int status;
    pid_t pid;
    ssize_t c;

    if (argc < 2) {
        printf("usage: %s PROGRAM\n", argv[0]);
        exit(1);
    }

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

    struct iovec src, dst;

    for (; map; map = map->hh.next) {

        if (strcmp(map->name, "[gprs]") == 0)
            continue;
        if (strcmp(map->name, "[fprs]") == 0)
            continue;
        if (strcmp(map->name, "[vdso]") == 0)
            continue;
        if (strcmp(map->name, "[vvar]") == 0)
            continue;
        if (strcmp(map->name, "[vsyscall]") == 0)
            continue;

        src = map->io;

        printf("%15s\t%7o\t%23lx\t%7ld\n",
               map->name, map->mode, (long) src.iov_base, src.iov_len);

        dst.iov_len = src.iov_len;
        dst.iov_base = calloc(dst.iov_len, sizeof(char));

        if ((c = process_vm_readv(pid, &dst, 1, &src, 1, 0)) != src.iov_len) {
            fprintf(stderr, "process_vm_readv: %m: returned %ld\n", c);
            __builtin_trap();
        }
    }


    return 0;
}
