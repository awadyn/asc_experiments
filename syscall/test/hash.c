#include <fann.h>
#include <openssl/sha.h>
#include <linux/perf_event.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <gmp.h>
#include <error.h>
#include <errno.h>
#include <assert.h>
#include <asc.h>

/* SHA-256 test vector on bit string of length zero is:
 *
 * e3b0c442 98fc1c14 9afbf4c8 996fb924 27ae41e4 649b934c a495991b 7852b855
 */
int asc_hook(pid_t pid[], ssv_t *X[], long L[], double A[])
{
    char buf[SHA256_DIGEST_LENGTH] = {0};
    double tic, toc, dt;
    int i, N = 15;
    long H;

    double r = A[0];

    if (r >= 0)
        return 0;

    ssv_t *x = X[0];
    ssv_t *y = X[1];
    ssv_t *z = X[2];

    tic = asc_timestamp();

    for (i = 0; i < N; i++) {
        H = asc_ssv_hash(x).uid;

        sprintf(buf, "%07lx", H >> 36 & 0xfffffff);

        assert(H == (long) 0xe3b0c44298fc1c14L);
        assert(strcmp(buf, "e3b0c44") == 0);

        H = asc_ssv_hash(y).uid;

        if (__builtin_popcount(i + 1) == 1 || i + 1 == N)
            printf("%7d\t%7s\t%07lx\t%7s\n",
                   i + 1, buf, H >> 36 & 0xfffffff,
                   asc_ssv_hash(z).sha[0]);
    }

    toc = asc_timestamp();

    dt = toc - tic;

    printf("%s: %.2g hash/s: %d hash calls in %f seconds (%.2g s/hash)\n",
           __FILE__, (3 * i)/dt, i, dt, dt/(3 * i));

    return 0;
}
