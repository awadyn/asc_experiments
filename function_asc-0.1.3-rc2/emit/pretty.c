#include <fann.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <sys/reg.h>
#include <linux/perf_event.h>
#include <error.h>
#include <gmp.h>
#include <limits.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/uio.h>
#include <stdio.h>
#include <assert.h>
#include <asc.h>

int asc_emit_pretty(pid_t pid[], ssv_t *X[], long L[], double A[])
{
#if ASC_TRACE
    fprintf(stderr, "DISPLAY\n");
#endif

    double dr;

    double r = A[0];
    double tic = A[3];
    double toc = A[4];

    long K = L[2];

    ssv_t *x = X[0];
    ssv_t *y = X[1];
    ssv_t *z = X[2];
    ssv_t *e = X[3];
    ssv_t *l = X[6];

    {
        if (r < 0)
            printf("%7s\t", "round");
        else
            printf("%7.0f\t", r);
    }

#if 0
    {
        if (r < 0)
            printf("%6s\t", "x_RIP");
        else
            printf("%06lx\t", asc_ssv_pc(x));
    }
#endif

    {
        if (r < 0)
            printf("%6s\t", "y_RIP");
        else
            printf("%06lx\t", asc_ssv_pc(y));
    }

#if 0
    {
        if (r < 0)
            printf("%7s\t", "hash(x)");
        else {
            char *h = asc_ssv_hash(x).sha[0];

            printf("%7s\t", h);

            if (r == 0) {
                assert(strcmp(h, "e3b0c44") == 0);
                assert(asc_ssv_pc(x) == 0);
            }
        }
    }
#endif

    {
        if (r < 0)
            printf("%7s\t", "hash(y)");
        else
            printf("%7s\t", asc_ssv_hash(y).sha[0]);
    }

    {
        if (r < 0)
            printf("%7s\t", "|e|");
        else
            printf("%7lu\t", asc_ssv_popcount(e));
    }

    {
        if (r < 0)
            printf("%7s\t", "|y-x|");
        else
            printf("%7lu\t", asc_ssv_popcount(z));
    }

    {
        if (r < 0)
            printf("%7s\t", "|y-u|");
        else
            printf("%7lu\t", asc_ssv_popcount(l));
    }

#if 1
    {
        if (r < 0)
            printf("%7s\t", "dR12");
        else {
            dr = asc_ssv_qword(y, R12) - asc_ssv_qword(x, R12);
#if 0
            static double mu;
            mu = 0.01 * dr + 0.99 * mu;
#endif
            printf("% #-7.1g\t", dr);
            errno = 0;
        }
    }
#endif

#if 0
    {
        if (r < 0)
            printf("%7s\t", "dRBX");
        else {
            dr = asc_ssv_qword(y, RBX) - asc_ssv_qword(x, RBX);
            printf(log10(fabs(dr)) > 6 ? "%7.0e\t" : "%7.0f\t", dr);
            errno = 0;
        }
    }
#endif

    {
        if (r < 0)
            printf("%7s\t", "dRAX");
        else {
            dr = asc_ssv_qword(y, RAX) - asc_ssv_qword(x, RAX);
            printf(log10(fabs(dr)) > 6 ? "%7.0e\t" : "%7.0f\t", dr);
            errno = 0;
        }
    }

#if 0
    {
        if (r < 0)
            printf("%7s\t", "dRCX");
        else {
            dr = asc_ssv_qword(y, RCX) - asc_ssv_qword(x, RCX);
            printf(log10(fabs(dr)) > 6 ? "%7.0e\t" : "%7.0f\t", dr);
            errno = 0;
        }
    }
#endif

#if 0
    {
        if (r < 0)
            printf("%7s\t", "hit");
        else {
            if (asc_ssv_popcount(l) == 0)
                printf("%+7d\t", 1);
            else
                printf("%7d\t", 0);
        }
    }
#endif

    {
        static double misses, queries;

        if (r < 0)
            printf("%7s\t", "miss%");
        else {
            queries++;
            if (r > 0 && asc_ssv_popcount(l) == 0) {
                printf("%+7.1f\t", misses / queries * 100);
            } else {
                misses++;
                printf("%7.1f\t", misses / queries * 100);
            }
        }
    }

    {
        if (r < 0)
            printf("%7s\t", "instr/s");
        else {
            double T = toc - tic;

            if (r > 2)
                assert(K > 0);

            if (r > 2)
                assert(T > 0);

            dr = (T > 0) ? K / T : 0;

            printf("%7.1e\t", dr);
        }
        errno = 0;
    }

    printf("\n");
    fflush(stdout);

    return 0;
}

int asc_hook(pid_t pid[], ssv_t *X[], long L[], double A[])
{
    return asc_emit_pretty(pid, X, L, A);
}
