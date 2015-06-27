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

#define ssvs(...) ((ssv_t *[]){__VA_ARGS__, 0})

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
    pid_t pid;

  START:
    if ((pid = asc_ptrace_start(argc - 1, argv + 1)) < 0)
        goto EXIT;
    goto EXIT;

  PANIC:
    fprintf(stderr, "PANIC: run aborted");
  EXIT:
    exit(errno);
}
