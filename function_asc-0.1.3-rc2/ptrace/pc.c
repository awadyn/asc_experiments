#include <fann.h>
#include <errno.h>
#include <linux/perf_event.h>
#include <error.h>
#include <gmp.h>
#include <stdint.h>
#include <sys/uio.h>
#include <sys/reg.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <linux/ptrace.h>
#include <asc.h>

long asc_ptrace_pc(pid_t pid)
{
    long nr;

    nr = ptrace(PTRACE_PEEKUSER, pid, 8 * RIP, 0);

    if (errno)
        return -errno;

    return nr;
}
