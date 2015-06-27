#include <asc.h>

double timestamp(struct timeval *tic)
{
    struct timeval toc, *now;

    if (tic == 0)
        now = &toc;
    else
        now = tic;

    gettimeofday(now, 0);

    return now->tv_sec + now->tv_usec / 1e6;
}
