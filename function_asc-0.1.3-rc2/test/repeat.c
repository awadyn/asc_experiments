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
    ssv_t *w;
    long Y, W;

    if (A[0] >= 0)
        return 0;

    ssv_t *y = X[1];

    puts(asc_ssv_hash(y).sha[0]);

    w = asc_ssv_gather(0, pid[1]);

    assert(w);

    puts(asc_ssv_hash(w).sha[0]);

    Y = asc_ssv_hash(y).uid;
    W = asc_ssv_hash(w).uid;

    printf("Y = %016lx\n", Y);
    printf("W = %016lx\n", W);

    assert(Y == W);

    asc_ssv_free(w);


    return 0;
}
