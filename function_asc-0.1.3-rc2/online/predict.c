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

double asc_online_predict(ssv_t *u, ssv_t const *y, ssv_t const *e, struct fann *h)
{
    mp_bitcnt_t i, j, k;
    fann_type *Pr;

    /* Always initialize prediction as current state.  */
    if ((asc_ssv_set(u, y)) == 0)
        return 0;

    /* Bail out early if no model created yet.  */
    if (h == 0)
        return 0;

    /* Bail out early if no excitations yet.  */
    if ((k = asc_ssv_popcount(e)) == 0)
        return 0;

    /* Stack-allocate dense input subvector for on-line prediction step.  */
    fann_type Y[k];

    /* Zero out dense input subvector.  */
    memset(Y, 0, k * sizeof(fann_type));

    for (j = asc_ssv_scan1(e, 0), i = 0; j < INFTY; j = asc_ssv_scan1(e, j+1)) {
        if (asc_ssv_tstbit(y, j))
            Y[i] = +1;
        i++;
    }

    /* Run model in forward mode.  */
    Pr = fann_run(h, Y);

    /* "Deproject" dense subvector `Pr' onto predicted state vector `u'.  */
    for (j = asc_ssv_scan1(e, 0), i = 0; j < INFTY; j = asc_ssv_scan1(e, j+1))
        if (Pr[i++] > 0.5)
            asc_ssv_combit(u, j);

    return 0;
}
