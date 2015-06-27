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

long asc_ssv_cmp(ssv_t const *x, ssv_t const *y)
{
    long i, k;

    if (x == 0)
        return -1;

    assert(x);
    assert(y);

#if 0
    printk("x = %lu y = %lu", x->N, y->N);
#endif

    if ((k = (x->N - y->N)))
        return k;

    for (i = 0; i < x->N; i++)
        if ((k = mpz_cmp(x->sub[i].z, y->sub[i].z)))
            return k;

    return 0;
}
