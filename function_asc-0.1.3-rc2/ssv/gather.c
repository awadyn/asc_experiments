#include <fann.h>
#include <errno.h>
#include <linux/perf_event.h>
#include <error.h>
#include <gmp.h>
#include <stdint.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <asc.h>

ssv_t *asc_ssv_gather(ssv_t *x, pid_t pid)
{
#if ASC_TRACE
    fprintf(stderr, "GATHER\n");
#endif
    int (*peek)(struct iovec *, struct iovec *, pid_t);
    struct iovec local, *remote;
    long i, bits;

    /* Allocate new memory only if necessary.  */
    if (x == 0)
        if ((x = asc_ssv_bind(0, pid)) == 0)
            goto failure;

    /* Walk through the dense subvectors of the sparse state vector `x'.  */
    for (i = 0; i < x->N; i++) {
        peek = x->sub[i].peek;

        if (peek) {
            remote = &x->sub[i].iov;
            bits = 8 * remote->iov_len;

            assert(bits > 0);

            /* Force memory allocation.  */
            mpz_setbit(x->sub[i].z, bits);

            local.iov_base = x->sub[i].z->_mp_d;
            local.iov_len = remote->iov_len;

            if (peek(&local, remote, pid))
                goto failure;

            mpz_clrbit(x->sub[i].z, bits);
        }
    }

    return x;

  failure:
    return 0;
}
