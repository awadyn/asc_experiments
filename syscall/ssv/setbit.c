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

ssv_t *asc_ssv_setbit(ssv_t *z, mp_bitcnt_t j)
{
    __mpz_struct *Z;
    mp_bitcnt_t l;
    int i;

    for (i = 0; i < z->N; i++) {
        if (asc_ssv_lo(z, i) <= j && j < asc_ssv_hi(z, i)) {
            l = asc_ssv_mod(j, z, i);
            Z = z->sub[i].z;
            mpz_setbit(Z, l);
            return z;
        }
#if 0
        printk("%lx %lx %lx", asc_ssv_lo(z, i)/8, j/8, asc_ssv_hi(z, i)/8);
#endif
    }

    assert(i < z->N);

    return 0;
}
