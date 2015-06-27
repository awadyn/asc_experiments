#include <fann.h>
#include <sys/uio.h>
#include <linux/perf_event.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <gmp.h>
#include <errno.h>
#include <error.h>
#include <assert.h>
#include <asc.h>

long asc_ssv_popcount(ssv_t const *x)
{
    long W = 0;
    int i;

    for (i = 0; i < x->N; i++)
        W += mpz_popcount(x->sub[i].z);

#if 0
    long J = 0;
    mp_bitcnt_t j;

    for (j = asc_ssv_scan1(x, 0); j < ULONG_MAX; j = asc_ssv_scan1(x, j + 1))
        J++;

    printk("W = %lu J = %lu", W, J);

    assert(W == J);
#endif

    return W;
}
