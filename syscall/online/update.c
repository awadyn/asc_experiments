#include <fann.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/ptrace.h>
#include <linux/perf_event.h>
#include <asm/unistd_64.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/param.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <fann.h>
#include <gmp.h>
#include <error.h>
#include <errno.h>
#include <assert.h>
#include <asc.h>

struct fann *asc_online_update(struct fann *h, struct fann_train_data *D,
                               ssv_t const *z, ssv_t const *x, ssv_t const *e)
{
    mp_bitcnt_t i, j, k;

    /* Bail out early if previous state is the zero vector.  */
    if (x == 0)
        return h;

    /* Bail out early if no excitations yet.  */
    if ((k = asc_ssv_popcount(e)) == 0)
        return h;

    /* Detect excitation count change.  */
    if (h && h->num_input != k) {
        fann_destroy(h);
        h = 0;
    }

    /* Resize the neural network.  */
    if (h == 0) {
        if ((h = fann_create_standard(3, k, 16, k)) == 0) {
            printk("panic");
            return 0;
        }

        fann_set_training_algorithm(h, FANN_TRAIN_INCREMENTAL);
        fann_set_activation_function_hidden(h, FANN_SIGMOID_SYMMETRIC);
        fann_set_activation_function_output(h, FANN_SIGMOID);
    }

    /* Stack-allocate dense subvectors for on-line SGD step.  */
    fann_type input[k];
    fann_type output[k];

    memset(input, 0, k * sizeof(fann_type));
    memset(output, 0, k * sizeof(fann_type));

    i = 0;

    for (j = asc_ssv_scan1(e, 0); j < INFTY; j = asc_ssv_scan1(e, j + 1)) {
        assert(i < k);

        /* Store into dense input subvector.  */
        if (asc_ssv_tstbit(x, j))
            input[i] = +1;

        /* Store into dense output subvector.  */
        if (asc_ssv_tstbit(z, j))
            output[i] = +1;

        i++;
    }

    if (fann_get_learning_rate(h) > 0) {
        /* Execute one on-line stochastic gradient descent (SGD) step.  */
        fann_train(h, input, output);

        /* Anneal the learning rate.  */
        fann_set_learning_rate(h, 0.99999 * fann_get_learning_rate(h));
    }

    if (D) {
        if (D->num_data == 0) {
            D->num_input = k;
            D->num_output = k;
        } else if (D->num_input != k) {
            for (i = 0; i < D->num_data; i++) {
                free(D->input[i]);
                free(D->output[i]);
            }

            D->num_data = 0;
            D->num_input = k;
            D->num_output = k;
        }

        struct fann_train_data X = {
            .errno_f = 0,
            .error_log = 0,
            .errstr = 0,
            .num_data = 1,
            .num_input = k,
            .num_output = k,
            .input = (fann_type *[]){input},
            .output = (fann_type *[]){output},
        };

#if 0
        printk("appending to %u-row training set", D->num_data);
#endif

        if (D->num_data < 1000 || drand48() < exp(-1.0/1e3 * D->num_data))
            fann_append_train_data(D, &X);
    }

    return h;
}
