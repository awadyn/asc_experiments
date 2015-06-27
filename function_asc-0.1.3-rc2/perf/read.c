#include <fann.h>
#include <errno.h>
#include <linux/perf_event.h>
#include <error.h>
#include <gmp.h>
#include <stdint.h>
#include <sys/uio.h>
#include <asc.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>

int asc_perf_read(uint64_t *counter, int fd)
{

    if (read(fd, counter, sizeof(uint64_t)) != sizeof(uint64_t)) {
        printk("%s: read failed on fd `%d'", __func__, fd);
        return -1;
    }

#if 0
    uint64_t reset = *counter;
errno = 0;
qq("%lu", *counter);
#endif

    *counter = *counter - 1;

if (1) {
    if (ioctl(fd, PERF_EVENT_IOC_RESET, 0)) {
            printk("ioctl");
            return -1;
    }
}

    return 0;
}
