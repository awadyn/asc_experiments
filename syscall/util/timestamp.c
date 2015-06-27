#include <fann.h>
#include <linux/perf_event.h>
#include <errno.h>
#include <error.h>
#include <gmp.h>
#include <stdint.h>
#include <sys/uio.h>
#include <asc.h>

#include <sys/time.h>
#include <stdlib.h>

double asc_util_timestamp(void)
{
    struct timeval now;

    gettimeofday(&now, 0);

    return now.tv_sec + now.tv_usec / 1e6;
}
