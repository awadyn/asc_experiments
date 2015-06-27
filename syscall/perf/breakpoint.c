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
#include <linux/hw_breakpoint.h>

int asc_perf_breakpoint(pid_t pid, void *address, uint64_t period)
{
    struct perf_event_attr pe[1] = {{0}};
    int fd;

    pe->exclude_kernel = 1;
    pe->exclude_idle = 1;
    pe->exclude_hv = 1;
    pe->pinned = 1;

    pe->size = sizeof(struct perf_event_attr);
    pe->type = PERF_TYPE_BREAKPOINT;
    pe->bp_type = HW_BREAKPOINT_X;
    pe->bp_len = sizeof(long);
    pe->bp_addr = (uint64_t) address;
    pe->sample_period = period;

#if 0
    printk("BREAKPOINT\t%llx\t%lld", pe->bp_addr, pe->sample_period);
#endif

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
