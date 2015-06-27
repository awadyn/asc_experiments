#include <assert.h>
#include <gmp.h>
#include <fann.h>
#include <math.h>
#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <asm/unistd.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/wait.h>
#include <sys/uio.h>

#ifndef PTRACE_SYSEMU_SINGLESTEP
#define PTRACE_SYSEMU_SINGLESTEP 32
#endif

/* Dense region of sparse state vector.  */
typedef struct ssv_subvector {
    struct iovec iov;
    int (*peek) (struct iovec * local, struct iovec * remote, pid_t pid);
    int (*poke) (struct iovec * local, struct iovec * remote, pid_t pid);
    mpz_t z;
} ssv_sub_t;

/* Sparse state vector.  */
typedef struct ssv {
    long N;
    struct {
        uint8_t SHA[256];
        char sha[1][8];
        long uid;
    };
    ssv_sub_t sub[];
} ssv_t;

#define ssvs(...) ((ssv_t *[]){__VA_ARGS__, 0})

pid_t asc_ptrace_start(int argc, char *argv[]);
ssv_t *asc_ssv_gather(ssv_t * x, pid_t pid);
long asc_ssv_cmp(ssv_t const *x, ssv_t const *y);
ssv_t asc_ssv_hash(ssv_t const *x);
long asc_ssv_qword(ssv_t const *z, long q);
int asc_ssv_swap(ssv_t * x, ssv_t * y);
long asc_ssv_pc(ssv_t const *x);
int asc_ssv_frees(ssv_t * X[]);
int asc_ssv_scatter(pid_t pid, ssv_t const *x);
ssv_t *asc_ssv_xor(ssv_t * z, ssv_t const *x, ssv_t const *y);
long asc_ssv_popcount(ssv_t const *x);
ssv_t *asc_ssv_ior(ssv_t * z, ssv_t const *x, ssv_t const *y);
struct fann *asc_update(struct fann *h, struct fann_train_data *D,
                        ssv_t const *z, ssv_t const *x, ssv_t const *e);
double asc_online_predict(ssv_t * u, ssv_t const *y,
                          ssv_t const *e, struct fann *h);

#define tr() fprintf(stderr, "%3u\n", __LINE__);

int main(int argc, char *argv[])
{
    double l, R = 0, H = 0, mse = 0.25;
    ssv_t *x, *y, *z, *e, *u, *r;
    struct fann *h = 0;
    long d, k, t = 0;
    int status;
    pid_t pid;

  START:
    if ((pid = asc_ptrace_start(argc - 1, argv + 1)) < 0)
        goto EXIT;
  ALLOCATE:
    if ((x = (asc_ssv_gather(0, pid))) == 0)
        goto PANIC;
    if ((y = (asc_ssv_gather(0, pid))) == 0)
        goto PANIC;
    if ((z = (asc_ssv_gather(0, pid))) == 0)
        goto PANIC;
    if ((u = (asc_ssv_gather(0, pid))) == 0)
        goto PANIC;
    if ((e = (asc_ssv_gather(0, pid))) == 0)
        goto PANIC;
    if ((r = (asc_ssv_gather(0, pid))) == 0)
        goto PANIC;
  CLEAR:
    asc_ssv_xor(x, x, x);
    asc_ssv_xor(z, z, z);
    asc_ssv_xor(u, u, u);
    asc_ssv_xor(e, e, e);
    asc_ssv_xor(r, r, r);
    goto TRANSITION;

  WAIT:
    if (waitpid(-1, &status, 0) != pid)
        goto PANIC;
  SWAP:
    asc_ssv_swap(x, y);
  GATHER:
    if ((asc_ssv_gather(y, pid)) == 0)
        goto PANIC;
  TRANSITION:
    if (ptrace(PTRACE_SYSEMU_SINGLESTEP, pid, 0, 0) < 0)
        goto PANIC;
    asc_ssv_xor(r, u, y);
  LOSS:
    l = asc_ssv_popcount(r);
  REGRET:
    if (t > 1)
        R += l;
  HITS:
    H += l == 0;
  DIFF:
    asc_ssv_xor(z, x, y);
  HAMMING:
    d = asc_ssv_popcount(z);
  EXCITE:
    if (t > 1)
        asc_ssv_ior(e, e, z);
  DEGREES:
    k = asc_ssv_popcount(e);
  UPDATE:
    h = asc_update(h, 0, z, x, e);
  MSE:
    if (h)
        mse = fann_get_MSE(h);
  PREDICT:
    asc_online_predict(u, y, e, h);
  READOUT:
    if (t++ == 0)
        printf("%7s\t%7s\t%7s\t%7s\t%7s\t%7s\n",
               "round", "|e|", "|y-x|", "|y-u|", "regret", "MSE");
    printf("%7ld\t%7ld\t%7ld\t%7.0f\t%.4f\t%.4f\t%7.3f\n",
           t, k, d, l, R / t, mse, 100 * (1 - H / t));
  LOOP:
    if (t > 10)
        goto EXIT;
    goto WAIT;

  PANIC:
    fprintf(stderr, "PANIC: run aborted: %m\n");
  EXIT:
    exit(errno);
}
