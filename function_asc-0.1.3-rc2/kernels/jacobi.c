/* This benchmark is part of the PolyBench/C 3.2 test suite.  */

#define HAVE_STDOUT 0

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static void kernel_jacobi_1d_imper(int tsteps, int n, double A[n], double B[n])
{
    register int t, i;

    for (t = 0; t < tsteps; t++) {

        for (i = 1; i < n - 1; i++)
            B[i] = 0.33333 * (A[i - 1] + A[i] + A[i + 1]);

        for (i = 0; i < n; i++)
            A[i] = B[i];
    }
}

static void init_array(int n, double A[n], double B[n])
{
    int i;

    for (i = 0; i < n; i++) {
        A[i] = 1 - 2 * (rand() / (double) RAND_MAX);
        B[i] = 1 - 2 * (rand() / (double) RAND_MAX);
    }
}

static void print_array(int n, double A[n])
{
#if HAVE_STDOUT
    int i;

    for (i = 0; i < n; i++)
        fprintf(stdout, "%15.12f\n", A[i]);
#endif
}

int main(int argc, char **argv)
{
    int i, n = 2, tsteps = 2;

    /* Size of 1-dimensional array.  */
    if (argc > 1)
        n = atoi(argv[1]);

    /* Number of timesteps.  */
    if (argc > 2)
        tsteps = atoi(argv[2]);

    /* Random seed.  */
    if (argc > 3)
        srand(atoi(argv[3]));

    double A[n], B[n];

    /* Initialize the input.  */
    init_array(n, A, B);

    if (tsteps > 0) {
        /* Run for a user-supplied number of time steps.  */
        kernel_jacobi_1d_imper(tsteps, n, A, B);
    } else if (tsteps == 0) {
        double a, b, epsilon = 0.0;

        /* Run until convergence is detected.  */
        do {
            a = A[n/2];
            kernel_jacobi_1d_imper(1, n, A, B);
            b = A[n/2];
#if 0
            printf("% 23.21f % 23.21f % 23.21f\n", a, b, b - a);
#endif
        } while (fabs(b - a) > epsilon);
    } else {
        /* Run forever.  */
        while (1) {
            kernel_jacobi_1d_imper(1, n, A, B);
#if 0
            printf("% 15.12f\n", A[1]);
#endif
        }
    }

    /* Print the answer.  */
    print_array(n, A);

    return 0;
}
