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

struct fann *asc_online_resume(ssv_t *e, char const *path)
{
    mp_bitcnt_t j, a;
    struct fann *h;
    unsigned i, n;

#if 0
    printk("resuming `%s' ...", path);
#endif

    if ((h = fann_create_from_file(path)) == 0)
        return 0;

    if (e == 0)
        return h;

    for (i = 0, n = 0; n < h->cascade_activation_steepnesses_count; i++) {

        for (a = 0; a < h->cascade_activation_steepnesses[n]; a++) {
            j = h->cascade_activation_steepnesses[n + 1 + a];
            asc_ssv_setbit(e, j + asc_ssv_lo(e, i));
        }

        n += 1 + h->cascade_activation_steepnesses[n];
    }

    return h;
}
