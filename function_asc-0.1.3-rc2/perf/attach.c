#define _GNU_SOURCE

#include <fann.h>
#include <errno.h>
#include <linux/perf_event.h>
#include <error.h>
#include <gmp.h>
#include <stdint.h>
#include <sys/uio.h>
#include <asc.h>

#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <asm/unistd_64.h>
#include <fcntl.h>
#include <sys/ioctl.h>

int asc_perf_attach(struct perf_event_attr *pe, pid_t pid)
{
    int fd;

    pe->size = sizeof(struct perf_event_attr);
    pe->exclude_kernel = 1;
    pe->exclude_idle = 1;
    pe->exclude_hv = 1;
    pe->pinned = 1;

    if (pe->bp_addr)
        pe->bp_len = sizeof(long);

    if ((fd = syscall(__NR_perf_event_open, pe, pid, -1, -1, 0)) < 0) {
        printk("perf_event_open failed");
        return -1;
    }

    if (fcntl(fd, F_SETFL, O_ASYNC) < 0) {
        printk("fcntl");
        return -1;
    }

    if (fcntl(fd, F_SETSIG, SIGSTOP) < 0) {
        printk("fcntl");
        return -1;
    }

    if (fcntl(fd, F_SETOWN, pid) < 0) {
        printk("fcntl");
        return -1;
    }

    if (ioctl(fd, PERF_EVENT_IOC_RESET, 0)) {
        printk("ioctl");
        return -1;
    }

    if (ioctl(fd, PERF_EVENT_IOC_ENABLE, 0)) {
        printk("ioctl");
        return -1;
    }

    return fd;
}
