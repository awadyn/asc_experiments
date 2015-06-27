#include <fann.h>
#include <sys/uio.h>
#include <linux/perf_event.h>
#include <stdlib.h>
#include <stdint.h>
#include <gmp.h>
#include <error.h>
#include <errno.h>
#include <asc.h>

int asc_ssv_swap(ssv_t *x, ssv_t *y)
{
    int i;

    if (x == 0)
        return -EINVAL;
    if (y == 0)
        return -EINVAL;
    if (x->N != y->N)
        return -EINVAL;

    for (i = 0; i < x->N; i++)
        mpz_swap(x->sub[i].z, y->sub[i].z);

    return 0;
}
