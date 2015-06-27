#include <asc.h>
#include <getopt.h>
#include <signal.h>
#include <sys/param.h>

struct fann_train_data *D;
unsigned subset;
unsigned column;
char *out;

/* Long command line options.  */
static struct option switches[] = {
    {"help", no_argument, 0, '?'},
    {"subset", required_argument, 0, 'S'},
    {"column", required_argument, 0, 'i'},
    {0, 0, 0, 0}
};

/* Short command line options.  */
static char const *letters = "+?S:i:";

int main(int argc, char **argv)
{
    struct fann_train_data *S;
    unsigned t;
    double a;
    int c, i; 

    /* Parse switched arguments.  */
    while ((c = getopt_long(argc, argv, letters, switches, &i)) != -1) {
        switch (c) {
        case '?':              /* Print usage statement and exit.  */
            if (optopt)
                goto failure;
            printf("`%s' is a weak learner for ASC\n", argv[0]);
            goto success;
        case 'S':
            if (sscanf(optarg, "%lg", &a) != 1) {
                diagnostic("illegal argument pair: `-%c %s'", c, optarg);
                goto failure;
            }
            subset = a;
            break;
        case 'i':
            if (sscanf(optarg, "%lg", &a) != 1) {
                diagnostic("illegal argument pair: `-%c %s'", c, optarg);
                goto failure;
            }
            column = a;
            break;
        default:
            goto failure;
        }
    }

    /* Parse non-switch arguments.  */
    for (i = optind; i < argc; i++) {
        if (strstr(argv[i], ".train") || strstr(argv[i], ".data")) {
            if ((S = fann_read_train_from_file(argv[i])) == 0)
                goto failure;

            if (D == 0)
                D = S;
            else {
                if ((D = fann_merge_train_data(D, S)) == 0)
                    goto failure;

                fann_destroy_train(S);
            }
        } else {
            diagnostic("illegal file: %s", argv[i]);
            goto failure;
        }
    }

    /* Bail out if no training data is available.  */
    if (D == 0) {
        fprintf(stderr, "%s: error: missing training data\n", argv[0]);
        goto failure;
    }

    if (subset > 0) {

        fann_shuffle_train_data(D);

        if ((S = fann_subset_train_data(D, 1, subset)) == 0) {
            diagnostic("fann_subset_train_data");
            goto failure;
        }
    } 
    
    if (column > 0) {

        for (t = 0; t < S->num_data - 1; t++)
            S->output[t][0] = S->output[t][column - 1];

        S->num_output = 1;
    }

    if (out == 0)
        out = "S.train";

    if (fann_save_train(S, out)) {
        diagnostic("fann save");
        goto failure;
    }

  success:
    return 0;

  failure:
    return 1;
}
