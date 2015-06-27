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

void show(ssv_t const *z, ssv_t const *x, ssv_t const *y)
{
    ssv_t const *I[3] = { z, x, y };
    long i, j, k;

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            k = asc_ssv_cmp(I[i], I[j]);
            printf("%5ld", k);
        }
        printf("\n");
    }
}

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

    tic = asc_timestamp();

    for (i = 0; i < N; i++) {

        assert(asc_ssv_cmp(z, z) == 0);
        assert(asc_ssv_cmp(z, x) != 0);
        assert(asc_ssv_cmp(z, y) == 0);

        assert(asc_ssv_cmp(x, z) != 0);
        assert(asc_ssv_cmp(x, x) == 0);
        assert(asc_ssv_cmp(x, y) != 0);

        assert(asc_ssv_cmp(z, z) == 0);
        assert(asc_ssv_cmp(z, x) != 0);
        assert(asc_ssv_cmp(z, y) == 0);

        k = asc_ssv_cmp(y, x);

        if (__builtin_popcount(i + 1) == 1 || i + 1 == N)
            printf("%7d\t%7ld\n", i + 1, k);
    }
    puts("");

    toc = asc_timestamp();

    dt = toc - tic;

    printf("%s: %.2g cmp/s: %d compares in %.2g seconds (%.2g s/cmp)\n",
           __FILE__, (10 * i)/dt, 10 * i, dt, dt/(10 * i));

    return 0;
}
