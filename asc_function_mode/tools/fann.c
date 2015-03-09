#include <getopt.h>
#include <fann.h>

/* Flags.  */
enum { HELP = 1, PARAMETERS = 2, GRAPHVIZ = 4 };

/* Command line options.  */
static struct option options[] = {
    {"help", no_argument, 0, 'h'},
    {"parameters", no_argument, 0, 'p'},
    {"graphviz", no_argument, 0, 'g'},
    {"threshold", required_argument, 0, 'w'},
    {0, 0, 0, 0}
};

int main(int argc, char *argv[])
{
    struct fann_connection *W;
    int c, i, C, flags = 0;
    char const *path[2];
    struct fann *nn[2];
    float w;

    /* Parse command line options.  */
    while ((c = getopt_long(argc, argv, "+hpgw", options, &i)) != -1) {
        switch (c) {
        case 'h':
            flags |= HELP;
            break;
        case 'p':
            flags |= PARAMETERS;
            break;
        case 'g':
            flags |= GRAPHVIZ;
            break;
        case 'w':
            if (sscanf(optarg, "%g", &w) != 1) {
                fprintf(stderr, "%s: illegal: `-w %s'", argv[0], optarg);
                goto failure;
            }
            break;
        default:
            goto failure;
        }
    }

    /* Open existing neural network file.  */
    if (argc - optind == 1) {
        /* Set path to existing neural network file.  */
        path[0] = argv[optind];

        /* Open existing neural network file.  */
        nn[0] = fann_create_from_file(path[0]);

        /* Bail out if error opening existing neural network file.  */
        if (nn[0] == 0)
            goto failure;
    } else {
        /* Bail out if we are confused about arguments.  */
        fprintf(stderr, "%s: missing or ambiguous arguments\n", argv[0]);
        goto failure;
    }

    if (flags & PARAMETERS) {
        /* Print neural network parameters.  */
        fann_print_parameters(nn[0]);
    } else if (flags & GRAPHVIZ) {
        C = fann_get_total_connections(nn[0]);

        W = calloc(C, sizeof(struct fann_connection));

        fann_get_connection_array(nn[0], W);

        /* Print header.  */
        printf("strict digraph G {\n\trankdir = LR;\n");

        /* Print edges.  */
        for (i = 0; i < C; i++)
            if (fabs(W[i].weight) > w)
                printf("\t%d -> %d [label=\"%+0.1f\"]\n",
                       W[i].from_neuron, W[i].to_neuron, W[i].weight);

        /* Print footer.  */
        printf("}\n");
    } else {
        /* Print usage statement.  */
        printf("`%s' is a FANN tool\n", argv[0]);
    }

    return 0;

  failure:
    return 1;
}
