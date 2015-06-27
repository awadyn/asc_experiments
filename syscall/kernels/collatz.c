/* Take any natural number i.  If i is even, divide it by 2 to get i / 2.
 * If i is odd, multiply it by 3 and add 1 to obtain 3i + 1.
 * Repeat the process indefinitely, halting if you reach 1 (a fixpoint).
 * The Collatz conjecture is that no matter what number i you start with,
 * you will always eventually reach 1.
 */

#include <stdio.h>
#include <stdlib.h>

static unsigned long step(unsigned long j)
{
    register unsigned long k = j;

    if ((k % 2) == 0)
        j = j / 2;
    else
        j = 3*j + 1;

    return j;
}

int main(int argc, char *argv[])
{
    unsigned long lo = 2, hi = 2;
    register unsigned long i;
    register unsigned long j;

    /* Set upper bound.  */
    if (argc == 2)
        hi = lo + atol(argv[1]);

    /* Set lower and upper bound.  */
    if (argc == 3) {
        lo = atol(argv[1]);
        hi = lo + atol(argv[2]);
    }

    /* Search for a counterexample to the Collatz conjecture.  */
    for (i = lo; i < hi; i++) {
        for (j = i; j > 1; j = step(j)) {
            /* This may be an infinite loop.  */
#if 0
            printf("%7lu\t%7lu\n", i, j);
#endif
        }
    }

    return i;
}
