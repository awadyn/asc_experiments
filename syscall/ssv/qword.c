#include <fann.h>
#include <errno.h>
#include <linux/perf_event.h>
#include <error.h>
#include <gmp.h>
#include <stdint.h>
#include <sys/uio.h>
#include <limits.h>
#include <asc.h>
#include <stdlib.h>

uint8_t asc_ssv_byte(ssv_t const *z, long q)
{
    mp_bitcnt_t const L = ULONG_MAX;
    __mpz_struct const *Z;
    mp_bitcnt_t k, l, lo; 
    int i;

    for (i = 0; i < z->N; i++) {
        lo = (mp_bitcnt_t) z->sub[i].iov.iov_base;
        Z = z->sub[i].z;

        for (l = mpz_scan1(Z, 0); l < L; l = mpz_scan1(Z, l + 1)) {
            k = l + lo * 8;                

            if (k / 8 == (mp_bitcnt_t) q) {
#if 0
                error(0, 0, "q = %lx k = %lx %lx", q, k, k / 8);
#endif
                return ((uint8_t *) Z->_mp_d)[l / 8];
            }
        }
    }

    return 0;
}

long asc_ssv_qword(ssv_t const *z, long q)
{
    mp_bitcnt_t const L = ULONG_MAX;
    __mpz_struct const *Z;
    mp_bitcnt_t k, l, lo; 
    int i;

    for (i = 0; i < z->N; i++) {
        lo = (mp_bitcnt_t) z->sub[i].iov.iov_base;
        Z = z->sub[i].z;

        for (l = mpz_scan1(Z, 0); l < L; l = mpz_scan1(Z, l + 1)) {
            k = l + lo * 8;                

            if (k / 64 == (mp_bitcnt_t) q) {
                return mpz_getlimbn(Z, l / 64);
            }
        }
    }

    return 0;
}
