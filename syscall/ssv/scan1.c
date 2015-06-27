#include <fann.h>
#include <linux/perf_event.h>
#include <stdio.h>
#include <error.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/uio.h>
#include <sys/param.h>
#include <gmp.h>
#include <errno.h>
#include <assert.h>
#include <asc.h>

mp_bitcnt_t asc_ssv_dim(ssv_t const *x, int i)
{
    return (mp_bitcnt_t) x->sub[i].iov.iov_len * 8;
}

mp_bitcnt_t asc_ssv_lo(ssv_t const *x, int i)
{
    return (mp_bitcnt_t) x->sub[i].iov.iov_base * 8;
}

mp_bitcnt_t asc_ssv_hi(ssv_t const *x, int i)
{
    return asc_ssv_lo(x, i) + asc_ssv_dim(x, i);
}

mp_bitcnt_t asc_ssv_mod(mp_bitcnt_t j, ssv_t const *x, int i)
{
    return (j > asc_ssv_lo(x, i)) ? j - asc_ssv_lo(x, i) : 0;
}

mp_bitcnt_t asc_ssv_scan1(ssv_t const *x, mp_bitcnt_t j)
{
    mp_bitcnt_t const L = ULONG_MAX;
    __mpz_struct const *X;
    mp_bitcnt_t k, l;
    int i, fastpath;

    assert(j < L);

    switch (j) {
    case 0 ... 8191:
        fastpath = 0;
        break;
    case 8192 ... 16383:
        fastpath = 1;
        break;
    default:
        fastpath = 2;
    }

    for (i = fastpath; i < x->N; i++) {
        //qq("%lx %lx", asc_ssv_lo(x, i) / 8, asc_ssv_hi(x, i) / 8);
        //qq("%lu %lu", asc_ssv_lo(x, i), asc_ssv_hi(x, i));

        if (j > asc_ssv_hi(x, i))
            continue;

        X = x->sub[i].z;

        l = (j > asc_ssv_lo(x, i)) ? j - asc_ssv_lo(x, i) : 0;

        for (l = mpz_scan1(X, l); l < L; l = mpz_scan1(X, l + 1)) {
            k = l + asc_ssv_lo(x, i);

            assert(k >= j);

            return k;
        }
    }

    return L;
}
