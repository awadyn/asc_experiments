#include <fann.h>
#include <errno.h>
#include <linux/perf_event.h>
#include <error.h>
#include <limits.h>
#include <stdint.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <gmp.h>
#include <assert.h>
#include <asc.h>

ssv_t *asc_ssv_clrbit(ssv_t *z, mp_bitcnt_t j)
{
    __mpz_struct *Z;
    mp_bitcnt_t l;
    int i;

    for (i = 0; i < z->N; i++) {
        if (asc_ssv_lo(z, i) <= j && j < asc_ssv_hi(z, i)) {
            l = asc_ssv_mod(j, z, i);
            Z = z->sub[i].z;
            mpz_clrbit(Z, l);
            return z;
        }
    }

    assert(i < z->N);

    return 0;
}

int asc_hook(pid_t pid[], ssv_t *X[], long L[], double A[])
{
    mp_bitcnt_t i, j, k, l, m;

    mp_bitcnt_t I[] = {
        1, 2, 3, 4, 5, 6, 1230, 1231, 1232, 8200, 8201, 8202, 8203, 8391,
        33557870, 33584520, 33584521,33584522, 1125899906809192,
        1125899906809193, 
        1125899906809194, 1125899906809195, 1125899906809196,
        1125899906809197, 1125899906809198, 1125899906809199,
        1125899906809782, 1125899906809783, 1125899906809784,
        1125899906809785, 1125899906809786, 1125899906809787,
        0,
    };

    if (A[0] >= 0)
        return 0;

    ssv_t *z = X[2];

    for (i = 0; (j = I[i]) > 0; i++) {
        k = asc_ssv_popcount(z);

        if (asc_ssv_tstbit(z, j)) {
            asc_ssv_clrbit(z, j);
            assert(asc_ssv_tstbit(z, j) == 0);
        } else {
            asc_ssv_setbit(z, j);
            assert(asc_ssv_tstbit(z, j) == 1);
        }

        l = asc_ssv_popcount(z);

        printk("|z|: %lu -> %lu", k, l);

        assert(l != k);

        asc_ssv_combit(z, j);

        m = asc_ssv_popcount(z);

        assert(m != l);
    }

    printk("OK");

    return 0;
}
