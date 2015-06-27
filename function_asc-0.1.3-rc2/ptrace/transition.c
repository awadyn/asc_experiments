#include <fann.h>
#include <errno.h>
#include <linux/perf_event.h>
#include <error.h>
#include <gmp.h>
#include <stdint.h>
#include <sys/uio.h>
#include <asc.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <linux/ptrace.h>
#include <assert.h>

int asc_ptrace_transition(pid_t pid, void *addr, double period, int fd)
{
#if ASC_TRACE
    fprintf(stderr, "TRANSITION\t%p %.0f\n", addr, period);
#endif

    if (addr == 0 && period == 1)
        return asc_ptrace_singlestep(pid, period, 0);
    else if (addr == 0 && period == -1)
        return asc_ptrace_blockstep(pid, 0);
    else if (addr == 0)
        return asc_ptrace_baseline(pid);

    fprintf(stderr, "%s: unsupported mode: addr = %p period = %.1f\n",
            __func__, addr, period);

    assert(0);

    return -EINVAL;
}
