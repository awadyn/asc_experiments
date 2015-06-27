#include <fann.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/ptrace.h>
#include <linux/perf_event.h>
#include <asm/unistd_64.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <fann.h>
#include <gmp.h>
#include <error.h>
#include <errno.h>
#include <assert.h>
#include <asc.h>

int asc_event(pid_t pid[], ssv_t *X[], long L[], double A[], struct fann *h[],
              int (*f[])(pid_t [], ssv_t *[], long [], double []))
{
    ssv_t *x = X[0], *y = X[1], *z = X[2], *e = X[3], *u = X[4], *l = X[6];
    struct fann_train_data D = {0};
    struct perf_event_attr pe[2];
    int i, fd, status;
    pid_t id;

    A[0] = -1;
    A[4] = asc_timestamp();

    for (i = 0; f[i]; i++)
        if (f[i](pid, X, L, A))
            return -1;

    A[0] = 0;

    for (i = 0; f[i]; i++)
        if (f[i](pid, X, L, A))
            return -1;

    void *addr = (void *)L[0];
    double period = A[2]; 

    /* Attach an instructions retired performance counter to master.  */
    memset(&pe[0], 0, sizeof(struct perf_event_attr));
    pe[0].type = PERF_TYPE_HARDWARE;
    pe[0].config = PERF_COUNT_HW_INSTRUCTIONS;
    if (addr == 0 && period > 1) {
        pe[0].sample_period = period;
        pe[0].precise_ip = 1;
    }
    if ((fd = asc_perf_attach(&pe[0], pid[0])) < 0)
        return -1;

    /* Go asynchronous.  */
    if (asc_ptrace_transition(pid[0], addr, period, fd))
        return -1;

    /* Event loop.  */
    for (A[0] = 1; A[0] < A[1]; A[0]++) {
        /* State: wait for an event to fire.  */
      WAIT:
#if ASC_TRACE
        fprintf(stderr, "WAIT\n");
#endif
        if ((id = waitpid(-1, &status, 0)) == -1) {
            printk("unexpected waitpid result");
            goto ERROR;
        }
        goto WAKE;

        /* State: determine status of whichever process just woke us.  */
      WAKE:
#if ASC_TRACE
        fprintf(stderr, "WAKE\n");
#endif
        if ((status >> 8) == (SIGTRAP | (PTRACE_EVENT_EXIT << 8))) {
#if ASC_TRACE
        fprintf(stderr, "TRACEEXIT\n");
#endif
            /* We were triggered by TRACEEXIT.  */
            goto GOODBYE;
        } else if (WSTOPSIG(status) == SIGSTOP) {
            /* Life is good.  */
        } else if (WSTOPSIG(status) != SIGTRAP) {
            /* Abnormal, so let's check to see if its a syscall.  */
            goto SIGNAL;
        }
        goto IDENTIFY;

        /* State: figure out if this is the master or a worker.  */
      IDENTIFY:
#if ASC_TRACE
        fprintf(stderr, "IDENTIFY\n");
#endif
        if (id != pid[0]) {
            /* It was the worker.  */
            goto WORKER;
        }
        /* It was the master.  */
        goto MASTER;

        /* State: it was the master, so fetch its new state vector.  */
      MASTER:
#if ASC_TRACE
        fprintf(stderr, "MASTER\n");
#endif
        if (asc_ssv_swap(x, y))
            goto ERROR;
        if ((asc_ssv_gather(y, pid[0])) == 0) {
            printk("could not gather state vector");
            goto ERROR;
        }
        if (asc_perf_read((uint64_t *) L + 1, fd) < 0)
            goto ERROR;
        goto QUERY;

        /* State: now see if we can fast-forward the master.  */
      QUERY: 
#if ASC_TRACE
        fprintf(stderr, "QUERY\n");
#endif
        asc_ssv_xor(l, y, u);
        if (asc_ssv_popcount(l) == 0) {
            /* Yes, we got a hit, so do something with it.  */
            goto HIT;
        }
        goto MISS;

        /* State: cache missed, go async again.  */
      MISS:
#if ASC_TRACE
        fprintf(stderr, "MISS\n");
#endif
        A[4] = asc_timestamp();
        L[2] += L[1];
        if (asc_ptrace_transition(pid[0], addr, period, fd)) {
            printk("unexpected transition result");
            goto ERROR;
        }
        goto FEATURE;

        /* State: on-line feature detection.  */
      FEATURE:
#if ASC_TRACE
        fprintf(stderr, "FEATURE\n");
#endif
        asc_ssv_xor(z, x, y);
        /* Calculate sparse cumulative excitation vector `e' from `z'.  */
        if (A[0] < 3) {
            /* Let the transient pass.  */
        } else
            asc_ssv_ior(e, e, z);
        goto UPDATE;

        /* State: on-line update.  */
      UPDATE:
#if ASC_TRACE
        fprintf(stderr, "UPDATE\n");
#endif
        h[1] = asc_update(h[0], &D, z, x, e);

        if (h[1] == 0 && h[0] != 0) {
            printk("neural network update failure");
            goto ERROR;
        }
        h[0] = h[1];
        h[1] = 0;
        goto PREDICT;

        /* State: on-line predict.  */
      PREDICT:
#if ASC_TRACE
        fprintf(stderr, "PREDICT\n");
#endif
        if (asc_online_predict(u, y, e, h[0]))
            goto ERROR;
        goto INSTALL;

        /* State: install prediction in worker context.  */
      INSTALL:
#if ASC_TRACE
        fprintf(stderr, "INSTALL\n");
#endif
        goto ISSUE;

        /* State: kick off asynchronous speculator.  */
      ISSUE:
#if ASC_TRACE
        fprintf(stderr, "ISSUE\n");
#endif
        goto STATUS;

        /* State: fast-forward the master.  */
      HIT:
#if ASC_TRACE
        fprintf(stderr, "HIT\n");
#endif
        /* Not yet implemented.  */
        goto MISS;

        /* State: it was the worker, so fetch its state vector.  */
      WORKER:
#if ASC_TRACE
        fprintf(stderr, "WORKER\n");
#endif
        printk("not yet implemented");
        goto ERROR;

        /* State: determine what's going on with this.  */
      SIGNAL:
#if ASC_TRACE
        fprintf(stderr, "SIGNAL\n");
#endif
        if (WSTOPSIG(status) != (SIGTRAP | 0x80)) {
            printk("unhandled signal");
            goto ERROR;
        }
        /* It is a syscall.  */
        goto SYSCALL;

        /* State: it is a system call, so decide which one.  */
      SYSCALL:
        switch (asc_ptrace_syscall(pid[0], status)) {
        case __NR_exit:
            /* We emulate this one.  */
#if ASC_TRACE
            fprintf(stderr, "EXIT\n");
#endif
            goto GOODBYE;
        default:
#if ASC_TRACE
            fprintf(stderr, "ILLEGAL\n");
#endif
            printk("unhandled system call");
            goto ERROR;
        }
        goto ERROR;

        /* State: exit failure.  */
      ERROR:
#if ASC_TRACE
        fprintf(stderr, "ERROR\n");
#endif
        /* Exit the event loop.  */
        break;

        /* State: exit success.  */
      GOODBYE:
#if ASC_TRACE
        fprintf(stderr, "GOODBYE\n");
#endif
        /* Exit the event loop.  */
        break;

        /* State: done, tell the world about it.  */
      STATUS:
#if ASC_TRACE
        fprintf(stderr, "STATUS\n");
#endif
        for (i = 0; f[i]; i++)
            if (f[i](pid, X, L, A))
                goto ERROR; 
    }

        /* State: save training data.  */
      WRITE:
#if ASC_TRACE
        fprintf(stderr, "WRITE\n");
#endif
    /* Write out the training set.  */
    if (D.num_data > 0) {
        unsigned t;

        if (fann_save_train(&D, "out.train") < 0)
            return -1;

        for (t = 0; t < D.num_data; t++) {
            free(D.input[t]);
            free(D.output[t]);
        }

        free(D.input);
        free(D.output);
    }

    return 0;
}
