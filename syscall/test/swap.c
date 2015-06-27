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
    ssv_t *x = X[0];
    ssv_t *y = X[1];

    if (A[0] >= 0)
        return 0;

    long a = asc_ssv_hash(x).uid;
    long b = asc_ssv_hash(y).uid;

    printf("%s\t%s\t", asc_ssv_hash(x).sha[0], asc_ssv_hash(y).sha[0]);

    assert(asc_ssv_hash(x).uid == a);
    assert(asc_ssv_hash(y).uid == b);

    asc_ssv_swap(x, y);

    assert(asc_ssv_hash(x).uid == b);
    assert(asc_ssv_hash(y).uid == a);

    printf("%s\t%s\t", asc_ssv_hash(x).sha[0], asc_ssv_hash(y).sha[0]);

    asc_ssv_swap(x, y);

    assert(asc_ssv_hash(x).uid == a);
    assert(asc_ssv_hash(y).uid == b);

    printf("%s\t%s\n", asc_ssv_hash(x).sha[0], asc_ssv_hash(y).sha[0]);

    return 0;
}
