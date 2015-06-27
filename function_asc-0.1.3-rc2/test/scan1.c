#include <fann.h>
#include <errno.h>
#include <linux/perf_event.h>
#include <error.h>
#include <limits.h>
#include <stdint.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <gmp.h>
#include <assert.h>
#include <asc.h>

int asc_ssv_scnprintf(ssv_t const *z)
{
    mp_bitcnt_t const J = ULONG_MAX;
    mp_bitcnt_t j, l;
    int roll = -1;

    for (j = asc_ssv_scan1(z, 0); j < J; j = asc_ssv_scan1(z, j + 1)) {
        if (roll < 0) {
            l = j;
            roll = 0;
        } else if (j - l == 1)
            roll++;
        else {
            if (roll > 1) {
                printf("%lu-%lu,", l, j);
                roll = -1;
            } else {
                printf("%lu,", l);
                l = j;
                roll = 0;
            }
        }
    }

    if (roll == 0)
        printf("%lu", l);
    else if (roll > 0)
        printf("%lu-%lu", l, l + roll);

    printf("\n");

    return 0;
}

int asc_hook(pid_t pid[], ssv_t *X[], long L[], double A[])
{
    mp_bitcnt_t const J = ULONG_MAX;
    double dt, tic, toc;
    mp_bitcnt_t j;
    int N = 50;
    long W, P;
    int i;

    if (A[0] >= 0)
        return 0;

    ssv_t *y = X[1];
    ssv_t *z = X[2];

    for (j = asc_ssv_scan1(y, 0), W = 0; j < J; j = asc_ssv_scan1(y, j + 1))
        W++;

    P = asc_ssv_popcount(z);

    tic = asc_timestamp();
    for (i = 0; i < N; i++) {
        for (j = asc_ssv_scan1(z, 0), W = 0; j < J; j = asc_ssv_scan1(z, j + 1))
            W++;

        if (W != P)
            printk("W = %lu P = %lu", W, P);

        assert(W == P);

        if (__builtin_popcount(i + 1) == 1 || i + 1 == N)
            printf("%7d\t%7ld\t%7ld\n", i + 1, W, P);
    }
    toc = asc_timestamp();
    dt = toc - tic;

    puts("");
    printf("%s:%.2g scan1/s: %d scan1 calls in %.2g seconds (%.2g s/scan1)\n",
           __FILE__, i/dt, i, dt, dt/i);

    return 0;
}
