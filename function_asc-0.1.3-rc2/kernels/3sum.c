/* The 3-SUBSET-SUM problem: given a multiset S of n integers, where each
 * integer is in the range [-N, +N], are there elements a, b, c in S such
 * that a + b + c = 0?
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static bool search(long const *S, long n)
{
    register long i, j, k, a, b, c;

    for (i = 0; i < n; i++, a = 0) {
        /* Reset elements.  */
        c = b = a = 0;

        /* Get first element.  */
        a = S[i];

        for (j = i + 1, a = S[i]; j < n; j++) {
            /* Get second element.  */
            b = S[j];

            for (k = j + 1; k < n; k++) {
                /* Get third element.  */
                c = S[k];

                /* Check whether the three elements satisfy the condition.  */
                if ((a + b + c) == 0) {
#if 1
                    printf("% 7ld\t% 7ld\t% 7ld\n", a, b, c);
#endif
                    return true;
                }
            }
        }
    }

    return false;
}

static void randomize(long *S, long n, long N)
{
    long i;

    for (i = 0; i < n; i++) {
        S[i] = (random() % (2 * N + 1)) - N;
#if 0
        printf("%+ld\n", S[i]);
#endif
    }
}

int main(int argc, char *argv[])
{
    /* Default parameters.  */
    unsigned seed = 1;
    long n = 2;
    long N = 2;

    /* Override default random seed.  */
    if (argc > 1)
        srandom(seed = atoi(argv[1]));

    /* Override default size of multiset.  */
    if (argc > 2)
        n = atol(argv[2]);

    /* Override default range of multiset elements.  */
    if (argc > 3)
        N = atol(argv[3]);

#if 0
    printf("seed = %d n = %ld N = %ld\n", seed, n, N);
#endif

    /* Allocate the multiset.  */
    long S[n];

    /* Initialize the multiset.  */
    randomize(S, n, N);

    /* Kick off the search.  */
    return search(S, n);
}
