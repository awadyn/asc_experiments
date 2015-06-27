#include <assert.h>
#include <gmp.h>
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

pid_t asc_ptrace_start(int argc, char *argv[]);
ssv_t *asc_ssv_gather(ssv_t * x, pid_t pid);
long asc_ssv_cmp(ssv_t const *x, ssv_t const *y);
ssv_t asc_ssv_hash(ssv_t const *x);
long asc_ssv_qword(ssv_t const *z, long q);
int asc_ssv_swap(ssv_t *x, ssv_t *y);
long asc_ssv_pc(ssv_t const *x);
int asc_ssv_frees(ssv_t *X[]);
int asc_ssv_scatter(pid_t pid, ssv_t const *x);

int main(int argc, char *argv[])
{
    int status;
    double t = 0;
    long i, nr, J = 2;
    pid_t id, pid, wid[J];
    ssv_t *X[3] = {0}, *W[J][3];

    if ((pid = asc_ptrace_start(argc - 1, argv + 1)) < 0)
        goto EXIT;
    if ((X[0] = asc_ssv_gather(0, pid)) == 0)
        goto PANIC;
    if ((X[1] = asc_ssv_gather(0, pid)) == 0)
        goto PANIC;
    assert(asc_ssv_cmp(X[0], X[1]) == 0);

    for (i = 0; i < J; i++) {
        if ((wid[i] = asc_ptrace_start(argc - 1, argv + 1)) < 0)
            goto PANIC;
        if ((W[i][0] = asc_ssv_gather(0, wid[i])) == 0)
            goto PANIC;
        if ((W[i][1] = asc_ssv_gather(0, wid[i])) == 0)
            goto PANIC;
        W[i][2] = 0;
        assert(asc_ssv_cmp(X[1], W[i][0]) == 0);
        assert(asc_ssv_cmp(X[1], W[i][1]) == 0);
    }
    goto FIRE;

  WAIT:
    if ((id = waitpid(-1, &status, 0)) < 0)
        goto PANIC;
    if (id != pid)
        goto WORKER;
  MASTER:
    if (WIFSTOPPED(status) == 0)
        goto PANIC;
  GATHER:
    asc_ssv_swap(X[0], X[1]);
    if ((asc_ssv_gather(X[1], pid)) == 0)
        goto PANIC;
  PRETTY:
    printf("%7.0f\t%.7s\t %06lx\t%.7s\t %06lx\t%.7s\t %06lx\n", t,
           asc_ssv_hash(X[1]).sha[0], asc_ssv_pc(X[1]),
           asc_ssv_hash(W[0][1]).sha[0], asc_ssv_pc(W[0][1]),
           asc_ssv_hash(W[1][1]).sha[0], asc_ssv_pc(W[1][1]));
  ASCERTAIN:
    if (WSTOPSIG(status) != SIGTRAP)
        goto SIGNAL;
  FIRE:
    t++;
    if (ptrace(PTRACE_SYSEMU_SINGLESTEP, pid, 0, 0) < 0)
        goto PANIC;
  PAYLOAD:
    if (asc_ssv_scatter(wid[0], X[1]))
        goto PANIC;
    if (asc_ssv_gather(W[0][1], wid[0]) == 0)
        goto PANIC;
    if (asc_ssv_gather(W[1][1], wid[1]) == 0)
        goto PANIC;
    goto WAIT;

  SIGNAL:
    if (WSTOPSIG(status) != (SIGTRAP | 0x80))
        goto PANIC;
  SYSCALL:
    nr = ptrace(PTRACE_PEEKUSER, pid, 8 * ORIG_RAX, 0);
    if (errno)
        goto PANIC;
    if (nr != __NR_exit)
        goto PANIC;
  FREE:
    asc_ssv_frees(X);
    for (i = 0; i < J; i++)
        asc_ssv_frees(W[i]);
  EXIT:
    exit(0);
  WORKER:
  PANIC:
    fprintf(stderr, "PANIC: run aborted");
    exit(1);
}
