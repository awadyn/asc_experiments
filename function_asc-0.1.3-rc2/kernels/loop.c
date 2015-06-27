/* A simple loop kernel.  */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    /* Default parameters.  */
    register long i, lo = 0, hi = 0;

    /* Override the default upper bound.  */
    if (argc == 2)
        hi = lo + atol(argv[1]);

    /* Override the default lower and upper bounds.  */
    if (argc == 3) {
        lo = atol(argv[1]);
        hi = lo + atol(argv[2]);
    }

    for (i = lo; i < hi || hi < 0; i++) {
        /* An empty loop.  */
    }

    return 0;
}
