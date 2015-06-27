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

ssv_t *asc_ssv_set(ssv_t *x, ssv_t const *y)
{
    long i;

    assert(x);
    assert(y);
    assert(x->N == y->N);

    for (i = 0; i < x->N; i++) {
        x->sub[i].iov.iov_base = y->sub[i].iov.iov_base;
        x->sub[i].iov.iov_len = y->sub[i].iov.iov_len;
        x->sub[i].peek = y->sub[i].peek;
        x->sub[i].poke = y->sub[i].poke;

#if 0
        assert(x->sub[i].z->_mp_d != y->sub[i].z->_mp_d);
#endif
        assert(asc_ssv_hi(x, i) == asc_ssv_hi(y, i));

        mpz_set(x->sub[i].z, y->sub[i].z);
    }

    return x;
}
