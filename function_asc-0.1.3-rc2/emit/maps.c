#include <fann.h>
#include <errno.h>
#include <linux/perf_event.h>
#include <error.h>
#include <gmp.h>
#include <limits.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/uio.h>
#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <assert.h>
#include <asc.h>

int asc_hook(pid_t pid[], ssv_t *X[], long L[], double A[])
{
    int i, M = 32;
    char buf[M][NAME_MAX];
    struct iovec iov[M];
    mode_t mode[M];
    char *name[M];
    ssv_t *d;

    if (A[0] >= 0)
        return 1;

    for (i = 0; i < M; i++)
        name[i] = buf[i];

    M = asc_maps_parse(iov, mode, name, M, pid[0]);

    assert(M != 0);

    for (i = 0; i < M; i++)
        printf("%23p\t%7lu\t    %c%c%c\t%s\n",
               iov[i].iov_base,
               iov[i].iov_len,
               mode[i] & S_IRUSR ? 'r' : '-',
               mode[i] & S_IWUSR ? 'w' : '-',
               mode[i] & S_IXUSR ? 'x' : '-',
               name[i]);

    d = asc_ssv_bind(0, pid[0]);

    assert(d);

    assert(d->N == M + 2);

    puts("");

    for (i = 0; i < d->N; i++)
        printf("%23p\t%7lu\n",
               d->sub[i].iov.iov_base,
               d->sub[i].iov.iov_len);

    asc_ssv_free(d);

    return 0;
}
