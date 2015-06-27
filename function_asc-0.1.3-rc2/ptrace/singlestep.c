#include <fann.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <linux/ptrace.h>
#include <linux/perf_event.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <gmp.h>
#include <error.h>
#include <errno.h>
#include <asc.h>

int asc_ptrace_singlestep(pid_t pid, double period, int *status)
{
#if ASC_TRACE
    fprintf(stderr, "SINGLESTEP\n");
#endif

    if (ptrace(PTRACE_SYSEMU_SINGLESTEP, pid, 0, 0) < 0) {
        printk("ptrace SYSEMU SINGLESTEP on pid `%d'", pid);
        goto failure;
    }

    if (status) {
        if ((waitpid(pid, status, 0)) == -1) {
            printk("%s: waitpid for pid `%d'", __func__, pid);
            goto failure;
        }
    }

    return 0;

  failure:
    return -1;
}
