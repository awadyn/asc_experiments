/* This benchmark is part of the PolyBench/C 3.2 test suite.  */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static void kernel_seidel_2d(int tsteps, int n, double A[n][n])
{
    int t, i, j;
    double a;

    for (t = 0; t <= tsteps - 1; t++) {
        for (i = 1; i <= n - 2; i++) {
            for (j = 1; j <= n - 2; j++) {
                a = 0;

                a += A[i - 1][j - 1];
                a += A[i - 1][j - 0];
                a += A[i - 1][j + 1];
                a += A[i - 0][j - 1];
                a += A[i - 0][j - 0];
                a += A[i - 0][j + 1];
                a += A[i + 1][j - 1];
                a += A[i + 1][j - 0];
                a += A[i + 1][j + 1];

                A[i][j] = a / 9.0;
            }
        }
    }

}

static void print_array(int n, double A[n][n])
{
#if 1
    int i, j;

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            fprintf(stdout, "%4.2f ", A[i][j]);
        }
        fprintf(stdout, "\n");
    }

    fprintf(stdout, "\n");
#endif
}

static void init_array(int n, double A[n][n])
{
    int i, j;

    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++)
            A[i][j] = 1 - 2 * (rand() / (double) RAND_MAX);
}

int main(int argc, char **argv)
{
    int n = 2, tsteps = 2;

    /* Size of 1-dimensional array.  */
    if (argc > 1)
        n = atoi(argv[1]);

    /* Number of timesteps.  */
    if (argc > 2)
        tsteps = atoi(argv[2]);

    /* Random seed.  */
    if (argc > 3)
        srand(atoi(argv[3]));

    double A[n][n];

    init_array(n, A);

    if (tsteps > 0) {
        /* Run for a user-supplied number of time steps.  */
        kernel_seidel_2d(tsteps, n, A);
    } else if (tsteps == 0) {
        double a, b, epsilon = 0.0;

        /* Run until convergence is detected.  */
        do {
            a = A[n/2][n/2];
            kernel_seidel_2d(1, n, A);
            b = A[n/2][n/2];
#if 0
            printf("% 23.21f % 23.21f % 23.21f\n", a, b, b - a);
#endif
        } while (fabs(b - a) > epsilon);
    } else {
        /* Run forever.  */
        while (1) {
            kernel_seidel_2d(1, n, A);
#if 0
            printf("% 15.12f\n", A[1]);
#endif
        }
    }

    /* Print the answer.  */
#if 0
    print_array(n, A);
#endif

    return 0;
}
