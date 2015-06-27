#include <fann.h>
#include <openssl/sha.h>
#include <linux/perf_event.h>
#include <error.h>
#include <gmp.h>
#include <limits.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/uio.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <asc.h>

int asc_emit_auxv(pid_t pid[], ssv_t *X[], long L[], double A[])
{
    mp_bitcnt_t j;

    if (A[0] < 0)
        return 0;

    ssv_t *y = X[1];

    for (j = asc_ssv_scan1(y, 0); j < INFTY; j = asc_ssv_scan1(y, j + 8))
        printf("%lx\n", j / 8);

    return 0;
}

int asc_hook(pid_t pid[], ssv_t *X[], long L[], double A[])
{
    return asc_emit_auxv(pid, X, L, A);
}
