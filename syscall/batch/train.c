#include <error.h>
#include <errno.h>
#include <fann.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

/* Generic error message macro.  */
#define diagnostic(...)                                                      \
do {                                                                         \
    error_at_line(0, errno, __FILE__, __LINE__, __VA_ARGS__);                \
} while (0)

int fann_save_safe(char const *path, struct fann const *net);

static enum fann_train_enum algorithm;
static struct fann_train_data *D;
static char *path = "kernel.net";
static unsigned layers[8];
static struct fann *h;
static int subset;
static int depth;

/* Long command line options.  */
static struct option switches[] = {
    {"help", no_argument, 0, '?'},
    {"subset", required_argument, 0, 's'},
    {"hidden", required_argument, 0, 'h'},
    {"rate", required_argument, 0, 'r'},
    {"epochs", required_argument, 0, 'e'},
    {"algorithm", required_argument, 0, 'a'},
    {0, 0, 0, 0}
};

/* Short command line options.  */
static char const *letters = "+?s:h:r:e:a:";

int asc_batch_train(int argc, char **argv)
{
    struct fann_train_data *S;
    int c, i, epochs = 20;
    float rate = 0.1;
    double a;

    /* Parse switched arguments.  */
    while ((c = getopt_long(argc, argv, letters, switches, &i)) != -1) {
        switch (c) {
        case '?':              /* Print usage statement and exit.  */
            if (optopt)
                goto failure;
            printf("`%s' is a weak learner for ASC\n", argv[0]);
            goto success;
        case 's':
            if (sscanf(optarg, "%lg", &a) != 1) {
                diagnostic("illegal argument pair: `-%c %s'", c, optarg);
                goto failure;
            }
            subset = a;
            break;
        case 'h':
            if ((depth = sscanf(optarg, "%dx%dx%dx%dx%dx%dx",
                                layers + 1, layers + 2, layers + 3, layers + 4,
                                layers + 5, layers + 6)) < 1) {
                diagnostic("illegal argument: `-%c %s'", c, optarg);
                goto failure;
            }
            break;
        case 'r':
            if (sscanf(optarg, "%lg", &a) != 1) {
                diagnostic("illegal argument pair: `-%c %s'", c, optarg);
                goto failure;
            }
            rate = a;
            break;
        case 'e':
            if (sscanf(optarg, "%lg", &a) != 1) {
                diagnostic("illegal argument pair: `-%c %s'", c, optarg);
                goto failure;
            }
            epochs = a;
            break;
        case 'a':
            if (strcmp(optarg, "incremental") == 0) {
                algorithm = FANN_TRAIN_INCREMENTAL;
            } else if (strcmp(optarg, "batch") == 0) {
                algorithm = FANN_TRAIN_BATCH;
            } else if (strcmp(optarg, "rprop") == 0) {
                algorithm = FANN_TRAIN_RPROP;
            } else if (strcmp(optarg, "quickprop") == 0) {
                algorithm = FANN_TRAIN_QUICKPROP;
            } else {
                diagnostic("illegal training algorithm: `%s'", optarg);
                goto failure;
            }
            break;
        default:
            goto failure;
        }
    }

    /* Parse non-switch arguments.  */
    for (i = optind; i < argc; i++) {
        if (strstr(argv[i], ".train")) {
            if ((S = fann_read_train_from_file(argv[i])) == 0)
                goto failure;

            if (D == 0) {
                D = S;
            } else {
                if ((D = fann_merge_train_data(D, S)) == 0)
                    goto failure;

                fann_destroy_train(S);
            }
        } else if (strstr(argv[i], ".net")) {

            path = argv[i];

            if ((h = fann_create_from_file(path)) == 0)
                goto failure;

            algorithm = fann_get_training_algorithm(h);
        } else {
            /* Gracefully ignore other suffixes.  */
        }
    }

    /* Bail out if no training data is available.  */
    if (D == 0) {
        fprintf(stderr, "%s: error: missing training data\n", argv[0]);
        goto failure;
    }

    /* Seed psuedorandom number generator.  */
    srand(getpid());

    if (depth > 0) {

        if (h) {
            diagnostic("contradictory arguments");
            goto failure;
        }

        layers[0] = D->num_input;
        layers[depth + 1] = D->num_output;

        if ((h = fann_create_standard_array(1 + depth + 1, layers)) == 0) {
            diagnostic("fann create");
            goto failure;
        }

        fann_set_training_algorithm(h, FANN_TRAIN_QUICKPROP);
        fann_set_activation_function_hidden(h, FANN_SIGMOID_SYMMETRIC);
        fann_set_activation_function_output(h, FANN_SIGMOID);
    }

    /* Bail out if no neural network is available.  */
    if (h == 0) {
        fprintf(stderr, "%s: error: missing neural network\n", argv[0]);
        goto failure;
    }

    fann_set_training_algorithm(h, algorithm);

    fann_set_train_stop_function(h, FANN_STOPFUNC_BIT);

    fann_set_bit_fail_limit(h, 0.5);

    if (fann_get_learning_rate(h) == 0)
        fann_set_learning_rate(h, 0.1);

    if (rate > 0) {
        fann_set_learning_rate(h, rate);
        printf("rate = %f\n", fann_get_learning_rate(h));
    }

    while (1) {

        if (subset > 0) {

            fann_shuffle_train_data(D);

            if ((S = fann_subset_train_data(D, 1, subset)) == 0) {
                diagnostic("fann_subset_train_data");
                goto failure;
            }

            fann_train_on_data(h, S, epochs, 1, 0);

            fann_destroy_train(S);

        } else
            fann_train_on_data(h, D, epochs, 1, 0);

        /* Smallest value FANN will recognize is 0.000001.  */
        fann_set_learning_rate(h, 0.000001);
        fann_save_safe(path, h);
        fann_set_learning_rate(h, rate);

        /* Stop training if zero bit errors.  */
        if (fann_get_bit_fail(h) == 0)
            break;
    }

  success:
    return 0;

  failure:
    return 1;
}
