#include <stdio.h>
#include <stdlib.h>


long step(long j)
{
	return (j%2 == 0)?j/2 : 3*j+1;
}


int main(int argc, char *argv[])
{
    unsigned long i, j, I[] = { 2, 4 };

    /* Set upper bound.  */
    if (argc == 2)
        I[1] = I[0] + atol(argv[1]);

    /* Set lower and upper bound.  */
    if (argc == 3) {
        I[0] = atol(argv[1]);
        I[1] = I[0] + atol(argv[2]);
    }

    /* Search for a counterexample to the Collatz conjecture.  */
    /*for (i = I[0]; i < I[1]; i++) {
        for (j = i; j > 1; ) {
            if (j % 2 == 0)
                j = j / 2;
            else
                j = 3 * j + 1;
        }
    }*/

	for(i = I[0]; i < I[1]; i ++)
		for(j = i; j > 1; j = step(j));

    return i;
}
