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
    mp_bitcnt_t j, k = -1;
    ssv_t *z = X[2];
    double d;

    for (j = k = asc_ssv_scan1(z, 0); j < INFTY; j = asc_ssv_scan1(z, j + 1)) {

        d = (j == k) ? j : j - k;

        printf("%g ", d);

        k = j;
    }
    printf("\n");

    return 0;
}
