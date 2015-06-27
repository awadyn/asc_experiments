#include <fann.h>
#include <errno.h>
#include <linux/perf_event.h>
#include <error.h>
#include <gmp.h>
#include <stdint.h>
#include <sys/uio.h>
#include <stdio.h>
#include <assert.h>
#include <asc.h>

int asc_hook(pid_t pid[], ssv_t *X[], long L[], double A[])
{
    double tic, toc, dt;
    int i, N = 800;
    ssv_t *z;

    if (A[0] >= 0)
        return 0;

    tic = asc_timestamp();
    for (i = 0; i < N; i++) {
        z = asc_ssv_calloc(i + 1);

        assert(z);

        assert(asc_ssv_popcount(z) == 0);

        if (__builtin_popcount(i + 1) == 1 || i + 1 == N)
            printf("%7d\t%7ld\n", i + 1, asc_ssv_popcount(z));

        asc_ssv_free(z);
    }
    printf("\n");
    toc = asc_timestamp();
    dt = toc - tic;

    printf("%s: %.2g calloc/s: %d allocates in %.2g seconds (%.2g s/calloc)\n",
           __FILE__, i/dt, i, dt, dt/i);

    return 0;
}
