#include <asc.h>
#include <unistd.h>

static char format[] = "`asc'"
 " executes and attempts to transparently speed up its program argument\n\n"
 "Usage: asc [OPTIONS] [EXCITATIONS] [EXAMPLES] [MODEL] PROGRAM [ARGUMENTS]\n\n"
 "Options:\n"
 " -h, --help                    Print this help statement and exit.\n"
 " -s, --sample=MODE[:MODIFIER]  Induce on-line learning rounds by MODE.\n"
 " -e, --metrics=METRIC[,...]    Print METRIC(s) to stdout at each round.\n"
 " -r, --rounds=LIMIT            Execute at most LIMIT on-line rounds.\n"
 "\n"
 "Modes:\n"
 "  singlestep  blockstep  breakpoint:ADDRESS[/PERIOD]  instructions:COUNT\n"
 "\n";

/* Print usage statement.  */
int help(void)
{
    if (write(1, format, sizeof format) < 0)
        __builtin_trap();

    return 0;
}
