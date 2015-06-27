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
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <asm/unistd.h>
#include <linux/hw_breakpoint.h>
#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/wait.h>
#include <sys/uio.h>

#include <sys/user.h>

/* representation of return address in a list of return addresses */
struct rtn{
        long addr;
        struct rtn *next;
};


typedef struct {
    bool singlestep;
    bool blockstep;
    bool breakpoint;
    bool function;
    bool instructions;
    struct {
        void *addr;
	void *rtn_addr;
        uint64_t period;
	struct rtn* rtn_stack;
    };
    double retired;
} asc_mode_t;

#ifndef PTRACE_SYSEMU
#define PTRACE_SYSEMU            31
#endif
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
double asc_online_predict(ssv_t *u, ssv_t const *y, ssv_t const *e, struct fann *h);
ssv_t *asc_ssv_set(ssv_t *x, ssv_t const *y);
int asc_online_save(char const *path, struct fann *h, ssv_t const *e);
struct fann *asc_online_resume(ssv_t *e, char const *path);
int fann_save_train_safe(char const *path, struct fann_train_data *data);
double asc_util_timestamp(void);
int asc_perf_breakpoint(pid_t pid, void *address, uint64_t period);
int asc_perf_instructions(pid_t pid, uint64_t period);
int fann_save_safe(char const *path, struct fann const *net);
int asc_batch_train(int argc, char **argv);
mp_bitcnt_t asc_ssv_scan1(ssv_t const *x, mp_bitcnt_t j);
uint8_t asc_ssv_byte(ssv_t const *z, long q);


/* add() and pop(): used to manipulate stack of return addresses in order to support recursion */
void add(struct rtn **rtn_stack, long rtn_addr);
struct rtn* pop(struct rtn **rtn_stack);


#define tr() fprintf(stderr, "%3u\n", __LINE__);

static volatile sig_atomic_t asc_target_pid;

static void handler(int nr)
{
    ptrace(PTRACE_SYSEMU_SINGLESTEP, asc_target_pid, 0, 0);
}

int main(int argc, char *argv[])
{
    double a, loss, tic, transient = 1, workers = 1, T = INFINITY;
    char net[PATH_MAX] = {0}, train[PATH_MAX] = {0};
    ssv_t *x, *y, *z, *r, *u, *v, *w, *e = 0;
    asc_mode_t *mode = (asc_mode_t []){{0}};
    char path[PATH_MAX], name[NAME_MAX];
    struct fann_train_data D = {0};
    uint64_t payoff, Payoff = 0;
    int c, status, fd[5] = {0};
    struct fann *h = 0;
    pid_t pid, wid;
    long nr, t = 0;


    long rip, rip_worker, rsp, rtn_address, err, err_worker;
    struct user_regs_struct gpr, gpr_worker;


    if (argc > 1) {
        if (strcmp(argv[1], "train") == 0) {
            return asc_batch_train(argc - 1, argv + 1);
        }
    }

    /* Long command line options.  */
    struct option switches[] = {
        {"address",      required_argument, 0, 'a'},
        {"blockstep",          no_argument, 0, 'b'},
        {"instructions", required_argument, 0, 'i'},
        {"function",     required_argument, 0, 'f'},
        {"jobs",         required_argument, 0, 'j'},
        {"rounds",       required_argument, 0, 'n'},
        {"singlestep",         no_argument, 0, 's'},
        {"transient",    required_argument, 0, 't'},
        {0, 0, 0, 0}
    };

    /* Short command line options.  */
    char const *letters = "+a:bi:f:j:n:st:";

    tic = asc_util_timestamp();

    /* Parse switched arguments.  */
    while ((c = getopt_long(argc, argv, letters, switches, 0)) != -1) {
        switch (c) {
        case 'a':
            if (sscanf(optarg, "%p/%lg", &mode->addr, &a) == 2) {
                /* Both the breakpoint and period supplied.  */
                mode->breakpoint = 1;
                mode->period = a;
            } else if (sscanf(optarg, "%p", &mode->addr) == 1) {
                /* Just the breakpoint supplied.  */
                mode->breakpoint = 1;
                mode->period = 1;
            } else {
                error(0, errno = EINVAL, "-%c `%s'", c, optarg);
                goto EXIT;
            }
            break;
        case 'b':
            mode->blockstep = 1;
            break;
        case 'f':
            if (sscanf(optarg, "%p/%lg", &mode->addr, &a) == 2) {
                /* Both the function address and the period supplied. */
                mode->function = 1;
                mode->period = a;
                mode->rtn_stack = NULL;
            } else if (sscanf(optarg, "%p", &mode->addr) == 1) {
                /* Just the function address supplied. */
                mode->function = 1;
		mode->singlestep = 0;
                mode->period = 1;
                mode->rtn_stack = NULL;
            } else {
                error(0, errno = EINVAL, "-%c `%s'", c, optarg);
                goto EXIT;
            }
            break;
        case 'j':              /* Number of jobs.  */
            if (sscanf(optarg, "%lg", &workers) != 1) {
                error(0, errno = EINVAL, "-%c `%s'", c, optarg);
                goto EXIT;
            }
            break;
        case 'i':
            if (sscanf(optarg, "%lg", &mode->retired) != 1) {
                error(0, errno = EINVAL, "-%c `%s'", c, optarg);
                goto EXIT;
            }
            mode->instructions = 1;
            break;
        case 'n':              /* Upper bound on number of rounds.  */
            if (sscanf(optarg, "%lg", &T) != 1) {
                error(0, errno = EINVAL, "-%c `%s'", c, optarg);
                goto EXIT;
            }
            break;
        case 's':
            mode->singlestep = 1;
            break;
        case 't':
            if (sscanf(optarg, "%lg", &transient) != 1) {
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
            printk("warning: could not find file `%s'", argv[optind]);
        }
    }

#if 0
    printk("path  = %s", path);
    printk("name  = %s", name);
    printk("net   = %s", net);
    printk("train = %s", train);
#endif
#if 0
    printk("period = %f", period);
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
                while (1) {
                    fann_set_learning_rate(h, 0.1);
                    fann_train_on_data(h, S, 25, 1, 0);

                    if (fann_save_safe(net, h) < 0)
                        goto PANIC;
                }
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

  COUNTERS:
    if ((fd[0] = asc_perf_instructions(pid, mode->retired)) < 0)
        goto PANIC;
    if ((fd[1] = asc_perf_instructions(wid, mode->retired)) < 0)
        goto PANIC;

  EVENTS:
    if (mode->breakpoint) {
        if ((fd[2] = asc_perf_breakpoint(pid, mode->addr, mode->period)) < 0)
            goto PANIC;
        if ((fd[3] = asc_perf_breakpoint(wid, mode->addr, mode->period)) < 0)
            goto PANIC;
    }
    if(mode->function) {
        if((fd[2] = asc_perf_breakpoint(pid, mode->addr, mode->period)) < 0)
          goto PANIC;
        if((fd[3] = asc_perf_breakpoint(wid, mode->addr, mode->period)) < 0)
          goto PANIC;
        if((fd[4] = asc_perf_breakpoint(pid, mode->rtn_addr, mode->period)) < 0)
          goto PANIC;
      }

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
    assert(asc_ssv_cmp(u, v) == 0);
    assert(asc_ssv_cmp(v, w) == 0);
    if (asc_ssv_cmp(u, y) != 0)
        printk("warning: auxv random bytes were not cleared");
    asc_ssv_xor(u, u, u);
    asc_ssv_xor(r, u, y);
    if (workers > 0) {
	if (mode->function) {
		if (ptrace(PTRACE_SYSEMU, wid, 0, 0) < 0)
			goto PANIC;
	} else {
        	if (ptrace(PTRACE_SYSEMU_SINGLESTEP, wid, 0, 0) < 0)
          		goto PANIC;
	}
    }
    goto RESIDUAL;

  WAIT:
    if ((nr = waitpid(-1, &status, 0)) < 0)
        assert(nr >= 0);

  DISPATCH:
    if (WSTOPSIG(status) == (SIGTRAP | 0x80)) {
        /* This is a TRACESYSGOOD event.  */
        if (nr == pid)
            goto SYSCALL;
        else if (nr == wid) {
            goto WORKER;
        } else {
            assert(0);
        }
    } else if (WSTOPSIG(status) == SIGTRAP) {
        if ((status >> 8) == (SIGTRAP | (PTRACE_EVENT_EXIT << 8))) {
            /* This is a TRACEEXIT event.  */
            if (nr == pid)
                goto SAVE;
            else if (nr == wid) {
                printk("lost our worker!");
                assert(0);
            } else {
                assert(0);
            }
        } else {
            /* This is just a normal trace trap.  */
            if (nr == pid)
                goto SWAP;
            else if (nr == wid) {
                goto WORKER;
            } else {
                assert(0);
            }
        }
    } else if (WSTOPSIG(status) == SIGSTOP) {
        /* This is probably a perf event.  */
        if (nr == pid)
            goto SWAP;
        else if (nr == wid) {
            goto WORKER;
        } else {
            assert(0);
        }
    } else {
        /* This is probably a wild speculation.  */
        if (nr == pid)
            assert(nr != pid);
        else if (nr == wid) {
            goto WORKER;
        } else {
            assert(0);
        }
    }
  SWAP:
    asc_ssv_swap(x, y);

  GATHER:

    if (mode->function) {
	//addr or rtn_addr
        if((err = ptrace(PTRACE_GETREGS, pid, 0, &gpr)) < 0)
          goto PANIC;
        rip = (long)gpr.rip;
	//addr:
        if(rip == (long)(mode->addr)) {
            rsp = ptrace(PTRACE_PEEKUSER, pid, 8*RSP, 0);
            rtn_address = ptrace(PTRACE_PEEKDATA, pid, rsp, 0);
            add(&(mode->rtn_stack), rtn_address);
            if((long)(mode->rtn_addr) != rtn_address)
              mode->rtn_addr = (void*)rtn_address;
            if(close(fd[4]) < 0)
              goto PANIC;
            if((fd[4] = asc_perf_breakpoint(pid, mode->rtn_addr, mode->period)) < 0)
              goto PANIC;
        }
	//rtn_addr:
        else {
            assert(rip == (long)(mode->rtn_addr));
            if(mode->rtn_stack != NULL) {
                pop(&(mode->rtn_stack));
                if(mode->rtn_stack != NULL)
                  if(mode->rtn_stack->addr != (long)(mode->rtn_addr))
                    mode->rtn_addr = (void*)(mode->rtn_stack->addr);
              }
            if(close(fd[4]) < 0)
              goto PANIC;
            if((fd[4] = asc_perf_breakpoint(pid, mode->rtn_addr, mode->period)) < 0)
              goto PANIC;
        }
    }

    if (mode->breakpoint && mode->instructions) {
        ioctl(fd[0], PERF_EVENT_IOC_DISABLE, 0);
        ioctl(fd[2], PERF_EVENT_IOC_ENABLE, 0);
        if (ptrace(PTRACE_SYSEMU, pid, 0, 0) < 0)
            goto PANIC;
        if (waitpid(pid, &status, 0) < 0)
            goto PANIC;
        ioctl(fd[2], PERF_EVENT_IOC_DISABLE, 0);
        ioctl(fd[0], PERF_EVENT_IOC_ENABLE, 0);
    }
  
    if ((asc_ssv_gather(y, pid)) == 0)
        goto PANIC;
  
    RESIDUAL:
    asc_ssv_xor(r, u, y);

  LOSS:
    if ((loss = asc_ssv_popcount(r)) == 0)
        goto TUNNEL;

  TRANSITION:
    if (mode->singlestep) {
        if (ptrace(PTRACE_SYSEMU_SINGLESTEP, pid, 0, 0) < 0)
            goto PANIC;
    } else if (mode->blockstep) {
        if (ptrace(PTRACE_SINGLEBLOCK, pid, 0, 0) < 0)
            goto PANIC;
    } else if (mode->breakpoint && mode->instructions) {
        if (ptrace(PTRACE_SYSEMU, pid, 0, 0) < 0)
            goto PANIC;
    } else if (mode->function) {
	if (ptrace(PTRACE_SYSEMU, pid, 0, 0) < 0)
	    goto PANIC;
    } else {
        if (ptrace(PTRACE_SYSEMU, pid, 0, 0) < 0)
            goto PANIC;
    }
    goto DIFF;
  TUNNEL:
    Payoff += payoff;
    if (asc_ssv_scatter(pid, v))
        goto PANIC;

    if (mode->function) {
        if((err = ptrace(PTRACE_GETREGS, pid, 0, &gpr)) < 0)
          goto PANIC;
        rip = (long)gpr.rip;

	if(rip == (long)(mode->addr)) {
	    if(mode->rtn_stack != NULL) {
		pop(&(mode->rtn_stack));
	        if(mode->rtn_stack != NULL) {
		    printf("RECURSIVE \n");
		    goto PANIC;
		}
	    }    
	}
    }


  DIFF:
    asc_ssv_xor(z, x, y);
  EXCITE:
    if (transient > 0 && t > transient)
        asc_ssv_ior(e, e, z);
	
  UPDATE:
    h = asc_online_update(h, &D, z, x, e);
  READOUT:
#if 1
    #include "display/default.i"
#else
    #include "display/factor.i"
#endif
  LOOP:
    if (t++ >= T)
        goto SAVE;
    if (loss == 0)
        goto SWAP;
    goto WAIT;

  SYSCALL:
    asc_ssv_swap(x, y);
    if ((asc_ssv_gather(y, pid)) == 0)
        goto PANIC;
    /* Syscall number is in RAX.
     * Arguments are in RDI, RSI, RDX, R10, R8 and R9.
     * Our return code should be placed in RAX.
     */

    long rdi = asc_ssv_qword(y, RDI);
    long rsi = asc_ssv_qword(y, RSI);
    long rdx = asc_ssv_qword(y, RDX);
#if 0
    error(0, 0, "RDI = %ld RSI = %lx RDX = %ld", rdi, rsi, rdx);
#endif
    switch (nr = ptrace(PTRACE_PEEKUSER, pid, 8 * ORIG_RAX, 0)) {
    case __NR_exit:
        goto SAVE;
    case __NR_write:
        /* We only support writing to stdout at the moment.  */
        if (rdi != 1) {
            error(0, ENOSYS, "write to fd %ld", rdi);
            goto EXIT;
        }
        long i;
        for (i = 0; i < rdx; i++) {
            uint8_t byte = asc_ssv_byte(y, rsi + i);
#if 0
            error(0, 0, "%x `%c'\n", byte, byte);
#endif
            if (write(1, &byte, 1) != 1) {
                error(0, errno, "write");
                goto PANIC;
            }
        }
        if (ptrace(PTRACE_POKEUSER, pid, 8 * RAX, rdx) < 0)
            goto PANIC;
        if (ptrace(PTRACE_SYSEMU, pid, 0, 0) < 0)
            goto PANIC;
        asc_ssv_set(y, x);
        goto WAIT;
    default:
        error(0, ENOSYS, "system call `%ld'", nr);
        goto EXIT;
    }

  WORKER:
    if (h == 0 || fann_get_MSE(h) > 0.1) {
        /* Too risky.  */
        asc_target_pid = wid;
        signal(SIGALRM, handler);
        alarm(1);
        goto WAIT;
    }
    if (mode->breakpoint && mode->instructions) {
        if (ioctl(fd[3], PERF_EVENT_IOC_ENABLE, 0))
            goto PANIC;
        if (ptrace(PTRACE_SYSEMU, wid, 0, 0) < 0)
            goto PANIC;
        if (waitpid(wid, &status, 0) != wid)
            goto PANIC;
        if (ioctl(fd[3], PERF_EVENT_IOC_DISABLE, 0))
            goto PANIC;
	if (read(fd[1], &payoff, sizeof(uint64_t)) != sizeof(uint64_t))
	    goto PANIC;
    }
    if (mode->function) {
	if((err_worker = ptrace(PTRACE_GETREGS, wid, 0, &gpr_worker)) < 0)
	  goto PANIC;
	rip_worker = (long)gpr.rip;
	assert(rip_worker == (long)(mode->addr));
    }
    if ((asc_ssv_gather(v, wid)) == 0)
        goto PANIC;
    /* Make sure that we made some forward progress.  */
    if (asc_ssv_cmp(v, w) != 0)
        asc_ssv_set(u, w);
    else
        asc_ssv_xor(u, u, u);
    /* We leave (u, v) behind as a tunnel.  */
    assert(asc_ssv_cmp(u, v) != 0);
  PREDICT:
    /* Now we use w to start a new tunnel. */
    asc_online_predict(w, y, e, h);
    asc_online_predict(w, w, e, h);
    if (asc_ssv_scatter(wid, w))
        goto PANIC;
  SPECULATE:
    ioctl(fd[1], PERF_EVENT_IOC_RESET, 0);
    if (mode->singlestep) {
        if (ptrace(PTRACE_SYSEMU_SINGLESTEP, wid, 0, 0) < 0)
            goto PANIC;
    } else if (mode->blockstep) {
        if (ptrace(PTRACE_SINGLEBLOCK, wid, 0, 0) < 0)
            goto PANIC;
    } else if (mode->breakpoint && mode->instructions) {
        if (ptrace(PTRACE_SYSEMU, wid, 0, 0) < 0)
            goto PANIC;
    } else if (mode->function) {
	if (ptrace(PTRACE_SYSEMU, wid, 0, 0) < 0)
	    goto PANIC;
    } else {
        if (ptrace(PTRACE_SYSEMU, wid, 0, 0) < 0)
            goto PANIC;
    }
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

/* adds a struct rtn to list of rtn structs
 * @param **rtn_stack: address of pointer to list of rtn structs
 * @param rtn_addr: return address to be added to the list
 */
void add(struct rtn **rtn_stack, long rtn_addr)
{
        struct rtn *new_rtn;
        new_rtn = (struct rtn*)malloc(sizeof(struct rtn));
        new_rtn->addr = rtn_addr;
        if(rtn_stack != NULL)
                new_rtn->next = *rtn_stack;
        else
                new_rtn->next = NULL;
        *rtn_stack = new_rtn;
}

/* removes the top (i.e: head) struct rtn from list of rtn structs
 * @param **rtn_stack: address of pointer to list of rtn structs
 */
struct rtn* pop(struct rtn **rtn_stack)
{
        struct rtn *top_rtn;
        if(rtn_stack != NULL)
        {
                top_rtn = *rtn_stack;
                *rtn_stack = (*rtn_stack)->next;
                top_rtn->next = NULL;
        }
        else
                top_rtn = NULL;
        return top_rtn;
}

