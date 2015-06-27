#include <fann.h>
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
#include <asc.h>

int asc_ptrace_blockstep(pid_t pid, int *status)
{
#if ASC_TRACE
    fprintf(stderr, "BLOCKSTEP\n");
#endif

    if (ptrace(PTRACE_SINGLEBLOCK, pid, 0, 0) < 0) {
        printk("ptrace SYSEMU SINGLEBLOCK on pid `%d'", pid);
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
