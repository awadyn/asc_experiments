#include <fann.h>
#include <sys/uio.h>
#include <linux/perf_event.h>
#include <stdlib.h>
#include <stdint.h>
#include <gmp.h>
#include <error.h>
#include <errno.h>
#include <assert.h>
#include <asc.h>

ssv_t *asc_ssv_calloc(long N)
{
    ssv_t *x;
    int i;

    assert(N > 0);

    if ((x = calloc(1, sizeof(ssv_t) + N * sizeof(ssv_sub_t))) == 0)
        goto failure;

    assert(x);

    x->N = N;

    for (i = 0; i < x->N; i++)
        mpz_init(x->sub[i].z);

    return x;

failure:
    return 0;
}
