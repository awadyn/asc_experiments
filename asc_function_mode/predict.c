#include <asc.h>

int predict(mpz_t u, struct fann **h, struct fann_train_data *D,
            mpz_t const x, mpz_t const e)
{
    long i, j, k;

    mpz_set(u, x);

    k = mpz_popcount(e);

    fann_type X[k], *U;

    if (k == 0)
        goto success;

    if (*h && h[0]->num_input < k) {
        fann_destroy(*h);

        *h = 0;

        D->num_data = 0;

        D->num_input = k;
        D->num_output = k;
    }

    if (*h == 0) {
        *h = fann_create_standard(3, k, 16, k, 0);

        fann_set_training_algorithm(*h, FANN_TRAIN_QUICKPROP);
        fann_set_activation_function_hidden(*h, FANN_SIGMOID_SYMMETRIC);
        fann_set_activation_function_output(*h, FANN_SIGMOID);
    }

    for (j = mpz_scan1(e, 0), i = 0; j > 0; j = mpz_scan1(e, j + 1), i++)
        X[i] = mpz_tstbit(x, j);

    U = fann_run(*h, X);

    for (j = mpz_scan1(e, 0), i = 0; j > 0; j = mpz_scan1(e, j + 1), i++) {
        if (U[i] > 0.5) {
            if (mpz_tstbit(u, j))
                mpz_clrbit(u, j);
            else
                mpz_setbit(u, j);
        }
    }

  success:
    return 0;
}
