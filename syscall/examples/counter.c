#define _GNU_SOURCE
#include <assert.h>
#include <getopt.h>
#include <gmp.h>
#include <fann.h>
#include <libgen.h>
#include <limits.h>
#include <math.h>
#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <asm/unistd.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/wait.h>
#include <sys/uio.h>

#ifndef PTRACE_SYSEMU_SINGLESTEP
#define PTRACE_SYSEMU_SINGLESTEP 32
#endif
#ifndef PTRACE_SINGLEBLOCK
#define PTRACE_SINGLEBLOCK       33
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

/* Error message macro.  */
#define printk(...)                                       \
do {                                                      \
    error_at_line(0, 0, __FILE__, __LINE__, __VA_ARGS__); \
} while (0)

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
struct fann *asc_online_update(struct fann *h, struct fann_train_data *D,
                               ssv_t const *z, ssv_t const *x, ssv_t const *e);
double asc_online_predict(ssv_t * u, ssv_t const *y,
                          ssv_t const *e, struct fann *h);
ssv_t *asc_ssv_set(ssv_t *x, ssv_t const *y);
int asc_online_save(char const *path, struct fann *h, ssv_t const *e);
struct fann *asc_online_resume(ssv_t *e, char const *path);
int fann_save_train_safe(char const *path, struct fann_train_data *data);
double asc_util_timestamp(void);

#define tr() fprintf(stderr, "%3u\n", __LINE__);

int main(int argc, char *argv[])
{
    double a, l, R = 0, H = 0, T = INFINITY;
    char net[PATH_MAX] = {0}, train[PATH_MAX] = {0};
    char path[PATH_MAX], name[NAME_MAX];
    ssv_t *x, *y, *z, *r, *u, *v, *w, *e = 0;
    struct fann_train_data D = {0};
    long nr, t = 0;
    struct fann *h = 0;
    pid_t pid, wid;
    int c, status;

    /* Long command line options.  */
    struct option switches[] = {
        {"rounds",       required_argument, 0, 'n'},
        {0, 0, 0, 0}
    };

    /* Short command line options.  */
    char const *letters = "+n:";

    /* Parse switched arguments.  */
    while ((c = getopt_long(argc, argv, letters, switches, 0)) != -1) {
        switch (c) {
        case 'n':              /* Upper bound on number of rounds.  */
            if (sscanf(optarg, "%lg", &T) != 1) {
                error(0, errno = EINVAL, "-%c `%s'", c, optarg);
                goto EXIT;
            }
            break;
        default:
            errno = EINVAL;
            goto EXIT;
        }
    }

    /* Parse non-switch arguments.  */
    for ( ; optind < argc; optind++) {
        if (access(argv[optind], R_OK) == 0) {
            char suffix[NAME_MAX];

            snprintf(path, PATH_MAX, "%s", argv[optind]);
            snprintf(name, NAME_MAX, "%s", basename(path));

            if (access(argv[optind], X_OK) == 0)
                break;

            if (sscanf(name, "%*[^.].%[^.]", suffix) == 1) {
                if (strcmp(suffix, "net") == 0) {
                    snprintf(net, PATH_MAX, "%s", path);
                } else if (strcmp(suffix, "train") == 0) {
                    snprintf(train, PATH_MAX, "%s", path);
                } else {
                    assert(0);
                }
            } else {
                assert(0);
            }
        } else {
            assert(0);
        }
    }

#if 0
    printk("path  = %s", path);
    printk("name  = %s", name);
    printk("net   = %s", net);
    printk("train = %s", train);
#endif

    if (argc == optind) {
        if (strlen(net) > 0) {
            if ((h = asc_online_resume(0, net)) == 0)
                goto PANIC;
            if (strlen(train) > 0) {
                struct fann_train_data *S;
                if ((S = fann_read_train_from_file(train)) == 0)
                    goto PANIC;
                printk("batch training `%s' on `%s' ...", net, train);
                fann_train_on_data(h, S, T < INFINITY ? T : 25, 1, 0);
                goto SAVE;
            } else
                assert(0);
        } else
            assert(0);
    }

  START:
    if ((wid = asc_ptrace_start(argc - optind, argv + optind)) < 0)
        goto EXIT;
    if ((pid = asc_ptrace_start(argc - optind, argv + optind)) < 0)
        goto EXIT;
  ALLOCATE:
    if ((x = (asc_ssv_gather(0, pid))) == 0)
        goto PANIC;
    if ((y = (asc_ssv_gather(0, pid))) == 0)
        goto PANIC;
    if ((z = (asc_ssv_gather(0, pid))) == 0)
        goto PANIC;
    if ((e = (asc_ssv_gather(0, pid))) == 0)
        goto PANIC;
    if ((r = (asc_ssv_gather(0, pid))) == 0)
        goto PANIC;
  CLEAR:
    asc_ssv_xor(x, x, x);
    asc_ssv_xor(z, z, z);
    asc_ssv_xor(e, e, e);
    if (strlen(net) > 0)
        if ((h = asc_online_resume(e, net)) == 0)
            goto PANIC;
    if (strlen(train) > 0) {
        assert(0);
    }
  KICKOFF:
    if ((u = (asc_ssv_gather(0, wid))) == 0)
        goto PANIC;
    if ((v = (asc_ssv_gather(0, wid))) == 0)
        goto PANIC;
    if ((w = (asc_ssv_gather(0, wid))) == 0)
        goto PANIC;
    asc_ssv_xor(u, u, u);
    asc_ssv_xor(r, u, y);
    if (ptrace(PTRACE_SYSEMU_SINGLESTEP, wid, 0, 0) < 0)
        goto PANIC;
    goto CHECK;

  WAIT:
    nr = waitpid(-1, &status, 0);
#if 0
    printk("nr = %ld wid = %d pid = %d", nr, wid, pid);
    sleep(1);
#endif
    if (nr == pid) 
        goto MASTER;
    else if (nr == wid)
        goto WORKER;
    goto PANIC;
  MASTER:
    if (WSTOPSIG(status) == SIGTRAP)
        goto SWAP;
    if (WSTOPSIG(status) == SIGSTOP)
        assert(0);
    if ((status >> 8) == (SIGTRAP | (PTRACE_EVENT_EXIT << 8)))
        assert(0);
    if (WSTOPSIG(status) == (SIGTRAP | 0x80))
        goto SYSCALL;
    assert(0);
  SWAP:
    asc_ssv_swap(x, y);
  GATHER:
    if ((asc_ssv_gather(y, pid)) == 0)
        goto PANIC;
  CHECK:
    asc_ssv_xor(r, u, y);
  LOSS:
    l = asc_ssv_popcount(r);
    if (l == 0)
        goto TUNNEL;
  TRANSITION:
    if (ptrace(PTRACE_SYSEMU_SINGLESTEP, pid, 0, 0) < 0)
        goto PANIC;
    goto REGRET;
  TUNNEL:
    goto TRANSITION;
  REGRET:
    if (t > 1)
        R += l;
  HITS:
    H += l == 0;
  DIFF:
    asc_ssv_xor(z, x, y);
  EXCITE:
    if (t > 1)
        asc_ssv_ior(e, e, z);
  UPDATE:
    h = asc_online_update(h, &D, z, x, e);
  READOUT:
    printf("%7ld\t", t);
    printf("%06lx\t", asc_ssv_pc(y));
    printf("%.7s\t", asc_ssv_hash(y).sha[0]);
    printf("%7lu\t", asc_ssv_qword(x, RCX));
    printf("%7lu\t", asc_ssv_qword(y, RCX));
    if (l == 0)
        printf("%+7.0f\t", l);
    else
        printf("%7.0f\t", l);
    printf("%.7s\t", asc_ssv_hash(u).sha[0]);
    if ((a = asc_ssv_qword(u, RCX)) > 1e7)
        printf("%7.2g\t", a);
    else
        printf("%7.0f\t", a);
    if ((a = asc_ssv_qword(v, RCX)) > 1e7)
        printf("%7.2g\t", a);
    else
        printf("%7.0f\t", a);
    printf("%.7s\t", asc_ssv_hash(v).sha[0]);
    printf("\n");
    fflush(stdout);
  LOOP:
    if (t++ >= T)
        goto SAVE;
    goto WAIT;

  SYSCALL:
    switch (nr = ptrace(PTRACE_PEEKUSER, pid, 8 * ORIG_RAX, 0)) {
    case __NR_exit:
        goto SAVE;
    default:
        error(0, ENOSYS, "system call `%lx'", nr);
        goto EXIT;
    }

  WORKER:
    if ((asc_ssv_gather(v, wid)) == 0)
        goto PANIC;
    asc_ssv_set(u, w);
  PREDICT:
    asc_online_predict(w, y, e, h);
    asc_online_predict(w, w, e, h);
#if 0
    if (t % 1 == 0) {
        printf("(%ld, %ld) (%ld, %ld) (%ld, ...) ",
                asc_ssv_qword(x, RCX), asc_ssv_qword(y, RCX),
                asc_ssv_qword(v, RCX), asc_ssv_qword(w, RCX),
                asc_ssv_qword(u, RCX));
        getchar();
    }
#endif
    if (asc_ssv_scatter(wid, w))
        goto PANIC;
    if (ptrace(PTRACE_SYSEMU_SINGLESTEP, wid, 0, 0) < 0)
        goto PANIC;
    goto WAIT;

  SAVE:
    if (h) {
        if (strlen(net) == 0)
            sprintf(net, "%s.net", name);
        if (asc_online_save(net, h, e) < 0)
            goto PANIC;
    }
    if (D.num_data > 0) {
        if (strlen(train) == 0)
            sprintf(train, "%s.train", name);
        if (fann_save_train_safe(train, &D) < 0)
            goto PANIC;
    }
    goto EXIT;

  PANIC:
    fprintf(stderr, "PANIC: run aborted: %m\n");
  EXIT:
    exit(errno);
}
