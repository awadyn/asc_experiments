#include <asc.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <unistd.h>

#include <sys/ptrace.h>
#include <asm/ptrace-abi.h>
#include <asm/unistd_64.h>
#include <sys/reg.h>
#include <sys/user.h>

#include <uthash.h>

/* Long command line options.  */
static struct option switches[] = {
    {"help", no_argument, 0, 'h'},
    {"sample", required_argument, 0, 's'},
    {"metrics", required_argument, 0, 'e'},
    {"rounds", required_argument, 0, 'r'},
    {0, 0, 0, 0}
};

/* Short command line options.  */
static char const *letters = "+hs:e:r:";

/* Parse command line options.  */
int options(struct machine *m, int argc, char *argv[])
{
    FILE *stream;
    double a;
    int c, i;
    long b;

    /* Parse switched arguments.  */
    while ((c = getopt_long(argc, argv, letters, switches, &i)) != -1) {
        switch (c) {
        case 'h':              /* Print usage statement and exit.  */
            m->flags |= HELP;
            goto success;
        case 's':              /* Execution mode induces on-line learning.  */
            if (strncmp(optarg, "si", 2) == 0) {
                /* Define on-line learning round by an instruction.  */
                m->mode |= SINGLESTEP;
            } else if (strncmp(optarg, "bl", 2) == 0) {
                /* Define on-line learning round by a basic block.  */
                m->mode |= BLOCKSTEP;
            } else if (strncmp(optarg, "br", 2) == 0) {
                /* Define on-line learning round by a breakpoint.  */
                m->mode |= BREAKPOINT;

                /* Parse the supplied breakpoint/count specification.  */
                if (sscanf(optarg, "%*[^:]:%lx/%lg", &b, &a) == 2) {
                    /* Both the breakpoint and period supplied.  */
                    m->address = b;
                    m->period = a;
                } else if (sscanf(optarg, "%*[^:]:%lx", &b) == 1) {
                    /* Just the breakpoint supplied.  */
                    m->address = b;
                    m->period = 1;
                } else {
                    diagnostic("missing or bad address in `%s'", optarg);
                    goto failure;
                }
            } else if (strncmp(optarg, "in", 2) == 0) {
                /* Define on-line learning round by number of instructions.  */
                m->mode |= INSTRUCTIONS;

                /* Parse the supplied instructions specification.  */
                if (sscanf(optarg, "%*[^:]:%lg", &a) == 1) {
                    m->instructions = a;
                } else {
                    diagnostic("illegal number of instructions: `%s'", optarg);
                    goto failure;
                }


            } else if(strncmp(optarg, "func", 4) ==0) {
		m->mode |= FUNCTION;
		if(sscanf(optarg, "%*[^:]:%lx", &b) == 1) {
		  m->address = b;
		  m->period = 1;
		  m->rtn_stack = NULL;
		} else {
			diagnostic("missing or bad function address in '%s'", optarg);
			goto failure;
		}
	} 

	else {
                diagnostic("illegal sampling mode: `%s'", optarg);
                goto failure;
            }
            break;
        case 'e':
            m->metrics = strdup(optarg);
            break;
        case 'r':              /* Execute no more than R rounds.  */
            if (sscanf(optarg, "%lg", &a) != 1) {
                diagnostic("illegal argument pair: `-%c %s'", c, optarg);
                goto failure;
            }
            m->rounds = a;
            break;
        default:
            goto failure;
        }
    }

    /* Parse non-switch arguments.  */
    for (i = optind; i < argc; i++) {
        if (strstr(argv[i], ".excite")) {

            if ((stream = fopen(argv[i], "r")) == 0) {
                diagnostic("fopen `%s'", argv[i]);
                goto failure;
            }

            mpz_init(m->excitations);

            if (mpz_inp_raw(m->excitations, stream) == 0) {
                diagnostic("mpz_inp_raw: failed to read `%s'", argv[i]);
                goto failure;
            }

            if (fclose(stream)) {
                diagnostic("fclose");
                goto failure;
            }

            /* Increment option count.  */
            optind++;
        } else if (strstr(argv[i], ".train")) {
            if ((m->data = fann_read_train_from_file(argv[i])) == 0)
                goto failure;

            /* Increment option count.  */
            optind++;
        } else if (strstr(argv[i], ".net")) {
            /* Load FANN neural network.  */
            if ((m->model = fann_create_from_file(argv[i])) == 0)
                goto failure;

            /* Increment option count.  */
            optind++;
        } else if (access(argv[i], X_OK) == 0) {
            /* Stop when we encounter an executable.  */
            break;
        } else {
            /* Bail out if we encounter an unrecognized file type.  */
            diagnostic("file `%s'", argv[i]);
            goto failure;
        }
    }

  success:
    return 0;

  failure:
    return -1;
}

