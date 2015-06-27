#include <fann.h>
#include <sys/reg.h>
#include <linux/perf_event.h>
#include <error.h>
#include <gmp.h>
#include <stdint.h>
#include <sys/uio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <asc.h>

long asc_ssv_pc(ssv_t const *x)
{
    assert(x);

    return asc_ssv_qword(x, RIP);
}
