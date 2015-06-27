#include <fann.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/ptrace.h>
#include <linux/perf_event.h>
#include <asm/unistd_64.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <fann.h>
#include <gmp.h>
#include <error.h>
#include <errno.h>
#include <assert.h>
#include <asc.h>

int asc_online_save(char const *path, struct fann *h, ssv_t const *e)
{
    mp_bitcnt_t j, k, l;
    __mpz_struct const *Z;
    fann_type *prev;
    int i;

#if 0
    printk("saving `%s' ...", path);
#endif

    if (e) {
        prev = h->cascade_activation_steepnesses;
        k = asc_ssv_popcount(e) + e->N;
        h->cascade_activation_steepnesses = calloc(k, sizeof(fann_type));
        h->cascade_activation_steepnesses_count = k;

        for (i = 0, l = 0; i < e->N; i++) {
            Z = e->sub[i].z;

            h->cascade_activation_steepnesses[l++] = mpz_popcount(Z);

            for (j = mpz_scan1(Z, 0); j < INFTY; j = mpz_scan1(Z, j + 1)) {
                h->cascade_activation_steepnesses[l++] = j;
            }
        }

        assert(l == (unsigned long) e->N + asc_ssv_popcount(e));
    }

    if (fann_save_safe(path, h) < 0) {
        assert(0);
        return -EINVAL;
    }

    if (e) {
        free(h->cascade_activation_steepnesses);

        h->cascade_activation_steepnesses = prev;
    }

    return 0;
}
