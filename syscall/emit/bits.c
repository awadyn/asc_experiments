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

int asc_hook(pid_t pid[], ssv_t *X[], long L[], double A[])
{
    mp_bitcnt_t j;
    ssv_t *y;

    if (A[0] >= 0)
        return 0;

    y = X[1];

    for (j = asc_ssv_scan1(y, 0); j < INFTY; j = asc_ssv_scan1(y, j + 1))
        printf("%16lu\n", j);

    return 0;
}
