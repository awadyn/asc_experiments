#include <fann.h>
#include <errno.h>
#include <linux/perf_event.h>
#include <error.h>
#include <gmp.h>
#include <stdint.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <linux/ptrace.h>
#include <asc.h>

int asc_ptrace_breakpoint(pid_t pid, void *addr, double period,
                          struct perf_event_attr *pe, int fd)
{
    if (ptrace(PTRACE_SYSEMU, pid, 0, 0) < 0) {
        printk("ptrace SYSEMU on pid `%d'", pid);
        goto failure;
    }

    return 0;

  failure:
    return -1;
}
