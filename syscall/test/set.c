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
#include <assert.h>
#include <asc.h>

int asc_hook(pid_t pid[], ssv_t *X[], long L[], double A[])
{
    double tic, toc, dt;
    int i, N = 600;
    long k;

    if (A[0] >= 0)
        return 0;

    ssv_t *x = X[0];
    ssv_t *y = X[1];
    ssv_t *z = X[2];

    assert(asc_ssv_cmp(y, x) != 0);
    assert(asc_ssv_cmp(z, y) == 0);

    tic = asc_timestamp();

    for (i = 0; i < N; i++) {
        if (__builtin_popcount(i + 1) == 1 || i + 1 == N)
            printf("%7d\t%7ld\n", i + 1, k = asc_ssv_cmp(z, x));

        asc_ssv_set(z, x);
        assert(asc_ssv_cmp(z, x) == 0);
        assert(asc_ssv_cmp(z, y) != 0);

        asc_ssv_set(z, y);
        assert(asc_ssv_cmp(z, x) != 0);
        assert(asc_ssv_cmp(z, y) == 0);
    }
    puts("");

    toc = asc_timestamp();

    dt = toc - tic;

    printf("%s: %.2g set/s: %d sets in %.2g seconds (%.2g s/set)\n",
           __FILE__, (10 * i)/dt, 10 * i, dt, dt/(10 * i));

    return 0;
}
