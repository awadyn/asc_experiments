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

ssv_t *asc_ssv_xor(ssv_t *z, ssv_t const *x, ssv_t const *y)
{
    int i;

    assert(x);
    assert(y);
    assert(x->N == y->N);

    if (z == 0) {
        if ((z = asc_ssv_calloc(x->N)) == 0)
            goto failure;

        if (asc_ssv_set(z, x) == 0)
            goto failure;
    }
#if 0
    errno = 0;
    long W = 0;
#endif
    for (i = 0; i < x->N; i++) {
        assert(asc_ssv_hi(x, i) == asc_ssv_hi(y, i));
        assert(asc_ssv_hi(z, i) == asc_ssv_hi(x, i));
#if 0
        printk("|x_%d| = %7ld", i, mpz_popcount(x->sub[i].z));
        printk("|y_%d| = %7ld", i, mpz_popcount(y->sub[i].z));
        printk("|z_%d| = %7ld", i, mpz_popcount(z->sub[i].z));
#endif
        mpz_xor(z->sub[i].z, x->sub[i].z, y->sub[i].z);
#if 0
        W += mpz_popcount(z->sub[i].z);

        printk("|Z_%d| = %7ld %7ld %7ld %7ld\n",
               i, mpz_popcount(z->sub[i].z), W,
               asc_ssv_popcount(z), asc_ssv_popcount(y));
#endif
    }

    return z;

failure:
    printk("%s: panic", __func__);
    __builtin_trap();
    return 0;
}

ssv_t *asc_ssv_ior(ssv_t *z, ssv_t const *x, ssv_t const *y)
{
    int i;

    assert(x);
    assert(y);
    assert(x->N == y->N);

    if (z == 0) {
        if ((z = asc_ssv_calloc(x->N)) == 0)
            return 0;

        if (asc_ssv_set(z, x) == 0)
            return 0;
    }

    for (i = 0; i < x->N; i++) {
        assert(asc_ssv_hi(x, i) == asc_ssv_hi(y, i));
        assert(asc_ssv_hi(z, i) == asc_ssv_hi(x, i));
#if 0
        printk("|x_%d| = %7ld", i, mpz_popcount(x->sub[i].z));
        printk("|y_%d| = %7ld", i, mpz_popcount(y->sub[i].z));
        printk("|z_%d| = %7ld", i, mpz_popcount(z->sub[i].z));
#endif
        mpz_ior(z->sub[i].z, x->sub[i].z, y->sub[i].z);
#if 0
        W += mpz_popcount(z->sub[i].z);

        printk("|Z_%d| = %7ld %7ld %7ld %7ld\n",
               i, mpz_popcount(z->sub[i].z), W,
               asc_ssv_popcount(z), asc_ssv_popcount(y));
#endif
    }

    return z;
}
