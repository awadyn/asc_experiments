#include <fann.h>
#include <errno.h>
#include <linux/perf_event.h>
#include <error.h>
#include <gmp.h>
#include <limits.h>
#include <sys/uio.h>
#include <stdlib.h>
#include <stdint.h>
#include <asc.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

ssv_t *asc_ssv_bind(ssv_t *x, pid_t pid)
{
    int i, M = 32;
    char buf[M][NAME_MAX];
    struct iovec iov[M];
    mode_t mode[M];
    char *name[M];

    if (pid <= 0)
        goto failure;

    for (i = 0; i < M; i++)
        name[i] = buf[i];

    if ((M = asc_maps_parse(iov, mode, name, M, pid)) == 0)
        goto failure;

    if (x == 0) {
        if ((x = asc_ssv_calloc(2 + M)) == 0)
            goto failure;
    } else if (x->N != 2 + M) {
        asc_ssv_free(x);

        if ((x = asc_ssv_calloc(2 + M)) == 0)
            goto failure;
    }

    /* General purpose registers.  */
    x->sub[0].iov.iov_base = (void *) 0;
    x->sub[0].iov.iov_len = 1024;
    x->sub[0].peek = asc_peek_gpr;
    x->sub[0].poke = asc_poke_gpr;

    /* Floating point and vector registers.  */
    x->sub[1].iov.iov_base = (void *) 1024;
    x->sub[1].iov.iov_len = 1024;
    x->sub[1].peek = asc_peek_fpr;
    x->sub[1].poke = asc_poke_fpr;

    /* Memory regions.  */
    for (i = 0; i < M; i++) {

        x->sub[2 + i].iov.iov_base = iov[i].iov_base;
        x->sub[2 + i].iov.iov_len = iov[i].iov_len;

        if (strcmp(name[i], "[vdso]") == 0) {
            /* Do not gather or scatter.  */
        } else if (strcmp(name[i], "[vvar]") == 0) {
            /* Do not gather or scatter.  */
        } else if (strcmp(name[i], "[vsyscall]") == 0) {
            /* Do not gather or scatter.  */
        } else if ((mode[i] & S_IWUSR) == 0) {
            /* Do not gather.  */
            x->sub[2 + i].peek = asc_peek_region;
        } else {
            x->sub[2 + i].peek = asc_peek_region;
            x->sub[2 + i].poke = asc_poke_region;
        }
    }

    return x;

  failure:
    return 0;
}
