#include <fann.h>
#include <sys/reg.h>
#include <errno.h>
#include <linux/perf_event.h>
#include <error.h>
#include <gmp.h>
#include <stdint.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <linux/ptrace.h>
#include <stdlib.h>
#include <stdio.h>
#include <asc.h>

int asc_ptrace_syscall(pid_t pid, int status)
{
#if ASC_TRACE
    fprintf(stderr, "SYSCALL\n");
#endif

    int nr;

    nr = ptrace(PTRACE_PEEKUSER, pid, 8 * ORIG_RAX, 0);

    if (errno) {
        printk("ptrace peekuser");
        return -errno;
    }

    return nr;
}
