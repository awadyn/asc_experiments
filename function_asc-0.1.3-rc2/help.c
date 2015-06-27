#include <fann.h>
#include <linux/perf_event.h>
#include <gmp.h>
#include <stdint.h>
#include <sys/uio.h>
#include <asc.h>

#include <unistd.h>

/* Usage statement.  */
static char format[] = "`asc'"
 " executes and attempts to transparently speed up its program argument\n\n"
 "Usage: asc [OPTIONS] [MODES] PROGRAM [...]\n\n"
 "Options:\n"
 " -b, --branches=N       Observe the state vector y every N branches.\n"
 " -i, --instructions=N   Observe the state vector y every N instructions.\n"
 " -h, --help             Print this help statement then exit.\n"
 " -n, --rounds=LIMIT     Execute at most LIMIT on-line rounds.\n"
 "\n";

/* Print usage statement.  */
int asc_help(void)
{
    if (write(1, format, sizeof format) < 0)
        __builtin_trap();

    return 0;
}
