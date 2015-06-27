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

int asc_ssv_scatter(pid_t pid, ssv_t const *x)
{
    int (*poke)(struct iovec *, struct iovec *, pid_t);
    struct iovec local, remote;
    long i;

    for (i = 0; i < x->N; i++) {
        if ((poke = x->sub[i].poke)) {
            /* Destination descriptor.  */
            remote.iov_base = x->sub[i].iov.iov_base;
            remote.iov_len = x->sub[i].iov.iov_len;

            /* Source descriptor.  */
            local.iov_base = x->sub[i].z->_mp_d;
            local.iov_len = remote.iov_len;

            // qq("%p %lu", remote.iov_base, remote.iov_len);

            /* Fire away.  */
            if (poke(&local, &remote, pid))
                return -EINVAL;
        }
    }

    return 0;
}
