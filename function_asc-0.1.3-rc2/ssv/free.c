#include <fann.h>
#include <errno.h>
#include <linux/perf_event.h>
#include <error.h>
#include <gmp.h>
#include <string.h>
#include <stdint.h>
#include <sys/uio.h>
#include <stdlib.h>
#include <assert.h>
#include <asc.h>

ssv_t *asc_ssv_free(ssv_t *x)
{
    long i;

    assert(x);

    for (i = 0; i < x->N; i++) {
        mpz_clear(x->sub[i].z);
        memset(&x->sub[i], 0, sizeof(ssv_sub_t));
    }

    x->N = 0;

    free(x);

    return 0;
}

int asc_ssv_frees(ssv_t *X[])
{
    int i;

    if (X == 0)
        return -EINVAL;

    for (i = 0; X[i]; i++)
        asc_ssv_free(X[i]);

    return 0;
}
