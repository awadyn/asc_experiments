#include <fann.h>
#include <errno.h>
#include <linux/perf_event.h>
#include <error.h>
#include <gmp.h>
#include <limits.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/uio.h>
#include <stdio.h>
#include <asc.h>

int asc_emit_pc(pid_t pid[], ssv_t *X[], long L[], double A[])
{
    double r = A[0];

    if (r < 0)
        return 0;

    ssv_t *x = X[0];
    ssv_t *y = X[1];

    printf("%7.0f\t%07lx\t%07lx\n", r, asc_ssv_pc(x), asc_ssv_pc(y));

    return 0;
}

int asc_hook(pid_t pid[], ssv_t *X[], long L[], double A[])
{
    return asc_emit_pc(pid, X, L, A);
}
