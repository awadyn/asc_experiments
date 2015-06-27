#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static long invert(long n)
{
    register long p;

    for (p = 3; p * p <= n; p += 2)
        if ((n % p) == 0)
            return p;

    return n;
}

int main(int argc, char *argv[])
{
    long p, n = 9223371994482243049;

    if (argc > 1)
        n = atol(argv[1]);

#if 0
    printf("n = %ld\n", n);
#endif
    
    p = invert(n);

    printf("%ld divides %ld\n", p, n);

    return p;
}
