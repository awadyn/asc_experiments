#include <fann.h>
#include <linux/perf_event.h>
#include <errno.h>
#include <error.h>
#include <gmp.h>
#include <stdint.h>
#include <sys/uio.h>
#include <asc.h>
#include <sys/ptrace.h>
#include <sys/user.h>

int asc_peek_gpr(struct iovec *local, struct iovec *remote, pid_t pid)
{
    struct user_regs_struct *gprs;

    if (ptrace(PTRACE_GETREGS, pid, 0, local->iov_base) < 0)
        goto failure;

    gprs = (struct user_regs_struct *) local->iov_base;

    /* Always set first reserved bit of EFLAGS.  */
    gprs->eflags |= 0x2;

    return 0;

  failure:
    return -1;
}
