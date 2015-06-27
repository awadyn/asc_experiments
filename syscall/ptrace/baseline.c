#include <fann.h>
#include <errno.h>
#include <linux/perf_event.h>
#include <error.h>
#include <gmp.h>
#include <stdint.h>
#include <sys/uio.h>
#include <asc.h>

#include <stdlib.h>
#include <sys/ptrace.h>
#include <linux/ptrace.h>

int asc_ptrace_baseline(pid_t pid)
{
#if ASC_TRACE
    fprintf(stderr, "BASELINE\n");
#endif

    if (ptrace(PTRACE_SYSEMU, pid, 0, 0) < 0) {
        printk("ptrace SYSEMU on pid `%d'", pid);
        goto failure;
    }

    return 0;

  failure:
    return -1;
}
