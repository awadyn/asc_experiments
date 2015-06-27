#include <fann.h>
#include <linux/perf_event.h>
#include <errno.h>
#include <error.h>
#include <gmp.h>
#include <sys/uio.h>
#include <stdint.h>
#include <asc.h>
#include <sys/ptrace.h>
#include <sys/user.h>

int asc_poke_gpr(struct iovec *local, struct iovec *remote, pid_t pid)
{
    if (ptrace(PTRACE_SETREGS, pid, 0, local->iov_base) < 0)
        goto failure;

    return 0;

  failure:
    return -1;
}
