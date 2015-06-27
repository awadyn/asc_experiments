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
    ssv_t *d;
    int i, j;
    long w;

    d = asc_ssv_xor(0, y, y);

    assert(d);

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            asc_ssv_xor(d, I[i], I[j]);
            w = asc_ssv_popcount(d);
            printf("%5ld", w);
        }
        printf("\n");
    }

    asc_ssv_free(d);
}

int asc_hook(pid_t pid[], ssv_t *X[], long L[], double A[])
{
    mp_bitcnt_t const J = ULONG_MAX;
    double tic, toc, dt;
    int i, N = 600;
    mp_bitcnt_t j;
    long k, W, U;
    ssv_t *d;

    if (A[0] >= 0)
        return 0;

    ssv_t *x = X[0];
    ssv_t *y = X[1];
    ssv_t *z = X[2];

    d = asc_ssv_xor(0, y, y);
    assert(asc_ssv_popcount(d) == 0);

    for (i = 0; i < 16384; i++) {
        mpz_setbit(d->sub[0].z, i);
        k = asc_ssv_popcount(d);
        assert(k == i + 1);
    }

    assert(asc_ssv_popcount(y) == asc_ssv_popcount(z));

    mp_bitcnt_t I[k];

    for (j = asc_ssv_scan1(z, 0), W = 0; j < J; j = asc_ssv_scan1(z, j + 1)) {
        I[W++] = j;
    }

    for (j = asc_ssv_scan1(y, 0), U = 0; j < J; j = asc_ssv_scan1(y, j + 1)) {
        assert(I[U++] == j);
    }

    assert(U == W);

    tic = asc_timestamp();

    for (i = 0; i < N; i++) {

        assert(asc_ssv_popcount(z) != 0);
        assert(asc_ssv_popcount(x) == 0);
        assert(asc_ssv_popcount(y) != 0);

        k = asc_ssv_popcount(z);

        if (__builtin_popcount(i + 1) == 1 || i + 1 == N)
            printf("%7d\t%7ld\t%7ld\n", i + 1, k, W);

        assert(k == W);
    }
    puts("");

    toc = asc_timestamp();

    dt = toc - tic;

    asc_ssv_free(d);

    printf("%s: %.2g cmp/s: %d compares in %.2g seconds (%.2g s/cmp)\n",
           __FILE__, (4 * i)/dt, 4 * i, dt, dt/(4 * i));

    return 0;
}
