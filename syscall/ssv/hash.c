#include <fann.h>
#include <errno.h>
#include <linux/perf_event.h>
#include <error.h>
#include <gmp.h>
#include <stdint.h>
#include <sys/uio.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/reg.h>
#include <openssl/sha.h>
#include <limits.h>
#include <string.h>
#include <asc.h>

ssv_t asc_ssv_hash(ssv_t const *x)
{
    mp_bitcnt_t const J = ULONG_MAX;
    SHA256_CTX sha;
    mp_bitcnt_t j;
    ssv_t X = {0};

    SHA256_Init(&sha);

    if (x) {
        for (j = asc_ssv_scan1(x, 0); j < J; j = asc_ssv_scan1(x, j + 1))
            SHA256_Update(&sha, &j, sizeof(mp_bitcnt_t));
    }

    SHA256_Final(X.SHA, &sha);

    X.uid =   0; X.uid |= X.SHA[0];
    X.uid <<= 8; X.uid |= X.SHA[1];
    X.uid <<= 8; X.uid |= X.SHA[2];
    X.uid <<= 8; X.uid |= X.SHA[3];
    X.uid <<= 8; X.uid |= X.SHA[4];
    X.uid <<= 8; X.uid |= X.SHA[5];
    X.uid <<= 8; X.uid |= X.SHA[6];
    X.uid <<= 8; X.uid |= X.SHA[7];

    snprintf(X.sha[0], 8, "%07lx", X.uid >> 36 & 0xfffffff);

    return X;
}
