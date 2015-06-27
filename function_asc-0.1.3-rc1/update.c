#include <asc.h>
#include <sys/reg.h>

int update(struct fann *h, struct fann_train_data *D,
           mpz_t const x, mpz_t const z, mpz_t const u, mpz_t const e)
{
    long i, j, k;

    /* Count the number of excited bits.  */
    k = mpz_popcount(e);

    if (k == 0)
        goto success;

    D->num_data++;

    D->num_input = k;
    D->num_output = k;

    D->input = realloc(D->input, D->num_data * sizeof(fann_type *));
    D->output = realloc(D->output, D->num_data * sizeof(fann_type *));

    D->input[D->num_data - 1] = calloc(k, sizeof(fann_type));
    D->output[D->num_data - 1] = calloc(k, sizeof(fann_type));

    for (j = mpz_scan1(e, 0), i = 0; j > 0; j = mpz_scan1(e, j + 1), i++) {
        D->input[D->num_data - 1][i] = mpz_tstbit(x, j);
        D->output[D->num_data - 1][i] = mpz_tstbit(z, j);
    }

    if (h && fann_get_learning_rate(h) > 0)
        fann_train(h, D->input[D->num_data - 1], D->output[D->num_data - 1]);

  success:
    return 0;
}
